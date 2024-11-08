TEST_ERROR_EXITCODE=5

is_job_active() {
	if ps -p $1 >& /dev/null; then
		return 0  # process is running
	else
		return 1  # process is dead
	fi
}

# on exit:
clean_up() {
	ARG=$?
    WS_PID=$1
	if [[ $WS_PID == 0 ]]; then
		exit $ARG
	fi
	if is_job_active "$WS_PID"; then
		kill -s INT "$WS_PID"
	else
		exit $ARG
	fi
}

# track fds does not affect exit code
vg() {
	local LOGFILE=$1
	local COMMAND=$2
	valgrind \
    --errors-for-leak-kinds=all \
	--leak-check=full \
	--show-error-list=yes \
	--show-leak-kinds=all \
	--trace-children=yes \
	--track-fds=yes \
	--track-origins=yes \
	--error-exitcode=$TEST_ERROR_EXITCODE \
	--log-file="$LOGFILE" \
	$COMMAND > /dev/null &
# --trace-children-skip="$(echo /bin/* /usr/bin/* /usr/sbin/* $(which norminette) | tr ' ' ',')"
# --suppressions="$RUNDIR/utils/minishell.supp"
}


has_open_fds_on_exit() {
	local LOGFILE=$1
	OPEN_FDS=$(grep "FILE DESCRIPTORS:" $LOGFILE | sed -E 's/(==[0-9]+== FILE DESCRIPTORS: ([0-9]+).*)/\2/')
	OPEN_STD_FDS=$(grep "FILE DESCRIPTORS:" $LOGFILE | sed -E 's/(==[0-9]+== FILE DESCRIPTORS: [0-9]+ open \(([0-9]+) std\).*)/\2/')
	INHERITED_FDS=$(grep -o '<inherited from parent>' $LOGFILE | wc -l)  # TODO check with children
	if (( $OPEN_FDS != $OPEN_STD_FDS + $INHERITED_FDS )); then
		return 0  # has open fds on exit
	fi
	return 1
}
