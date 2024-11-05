set -e

# include test_utils.sh
SCRIPT_DIR=$(dirname "$0")
source $SCRIPT_DIR/test_utils.sh

WS_RUN_COMMAND=$1
VALGRIND_LOGFILE=$2

########################_SCRIPT_START_###############################

# Start ws in background
vg "$VALGRIND_LOGFILE" "$WS_RUN_COMMAND"
WS_PID=$!
sleep 4  # wait for server to start

if ! is_job_active "$WS_PID"; then
	>&2 echo "Error starting server"
	exit $TEST_ERROR_EXITCODE
fi
echo "Server sucessfully started"
trap "clean_up $WS_PID" EXIT  # kill process if exit with error

# Stop server
kill -s INT $WS_PID
wait $WS_PID
VAL_EXIT_CODE=$?

# Check ws exitcode
if [[ $VAL_EXIT_CODE == $TEST_ERROR_EXITCODE ]]; then
    exit $TEST_ERROR_EXITCODE
fi
echo "Server stopped"

# Check ws fd-s
if has_open_fds_on_exit "$VALGRIND_LOGFILE"; then
	exit $TEST_ERROR_EXITCODE
fi
