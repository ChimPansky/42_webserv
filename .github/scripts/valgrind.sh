COMMAND=$1
LOGFILE=$2

ERROR_EXITCODE=5

# track fds does not affect exit code
valgrind --error-exitcode=$ERROR_EXITCODE \
    --errors-for-leak-kinds=all \
	--leak-check=full \
	--show-error-list=yes \
	--show-leak-kinds=all \
	--trace-children=yes \
	--track-fds=yes \
	--track-origins=yes \
	--log-file="$LOGFILE" \
    $COMMAND &

	# --trace-children-skip="$(echo /bin/* /usr/bin/* /usr/sbin/* $(which norminette) | tr ' ' ',')"
	# --suppressions="$RUNDIR/utils/minishell.supp"

VAL_PID=$!

sleep 5

kill -s INT $VAL_PID

wait $VAL_PID

VAL_EXIT_CODE=$?

if [[ $VAL_EXIT_CODE == $ERROR_EXITCODE ]]; then
    exit $ERROR_EXITCODE
fi

OPEN_FDS=$(grep "FILE DESCRIPTORS:" $LOGFILE | sed -E 's/(==[0-9]+== FILE DESCRIPTORS: ([0-9]+).*)/\2/')
OPEN_STD_FDS=$(grep "FILE DESCRIPTORS:" valgr.out | sed -E 's/(==[0-9]+== FILE DESCRIPTORS: [0-9]+ open \(([0-9]+) std\).*)/\2/')

# +1 is valgrind log file
if (( $OPEN_FDS != $OPEN_STD_FDS + 1 )); then
    exit $ERROR_EXITCODE
fi
