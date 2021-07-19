
#../websocketd --devconsole --port=8080 --address=$(hostname  -I) --origin=http://raspberryip.dev  sudo bin/remote
../websocketd --devconsole --port=8080 --address=$(hostname  -I) --header="Content-Type: application/json\nAccept: application/json" sudo bin/remote
