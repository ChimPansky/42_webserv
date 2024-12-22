set -e
# set -x

# include test_utils.sh
SCRIPT_DIR=$(dirname "$0")
source $SCRIPT_DIR/test_utils.sh

WS_RUN_COMMAND=$1
VALGRIND_LOGFILE=$2


########################_SCRIPT_START_###############################

# Start ws in background
vg "$VALGRIND_LOGFILE" "$WS_RUN_COMMAND"
WS_PID=$!
sleep 1  # wait for server to start

if ! is_job_active "$WS_PID"; then
	>&2 echo "Error starting server"
	exit $TEST_ERROR_EXITCODE
fi
echo "Server sucessfully started"
trap "clean_up $WS_PID" EXIT  # kill process if exit with error

# Send random rq with nc
# TODO: get host and port from cfg with grep?
RESPONSE=$(echo -e "GET /cgi-bin/hello_world.sh HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n" | nc -N -w 1 localhost 5500)
sleep 5  # wait for response

# Extract the status code from the response
STATUS_CODE=$(echo "$RESPONSE" | head -n 1 | awk '{print $2}')

# Check if the status code is what you expect (e.g., 200)
EXPECTED_STATUS_CODE="200"
if [[ "$STATUS_CODE" != "$EXPECTED_STATUS_CODE" ]]; then
    echo "Unexpected status code: $STATUS_CODE"
    echo "Expected: $EXPECTED_STATUS_CODE"
    exit $TEST_ERROR_EXITCODE
fi
echo "Status code is $STATUS_CODE as expected"

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

# Check response not empty, later compare with presaved
if [[ -z "$RESPONSE" ]]; then
    echo "response is empty"
	exit $TEST_ERROR_EXITCODE
fi
