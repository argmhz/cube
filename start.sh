ADDR=$(hostname -I)
#../websocketd --devconsole --port=8080 --address=$(hostname  -I) --origin=http://raspberryip.dev  sudo bin/remote
#../websocketd --devconsole --port=8080 --address=$(hostname  -I) --header="Content-Type: application/json\nAccept: application/json" sudo bin/remote
# echo "connection to ${ADDR}"
# ../websocketd --devconsole --port=8080 --address=$ADDR sudo bin/remote &
# sleep 5
# echo "pinging " http://${ADDR//[[:space:]]/}:8080/
# curl --include --no-buffer --header "Connection: Upgrade" --header "Upgrade: websocket" --header "Sec-WebSocket-Version: 13" --header "Sec-WebSocket-Key: SGVsbG8sIHdvcmxkIQ==" \
#     http://${ADDR//[[:space:]]/}:8080/ &
# # curl ${ADDR//[[:space:]]/}:8080/
# sleep 2
# sudo killall curl
