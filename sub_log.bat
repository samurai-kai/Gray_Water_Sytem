docker exec -it mosquitto mosquitto_sub -u mqttuser -P PASSWORD_HERE -t "graywater/clean/#" -v

docker exec -it mosquitto mosquitto_sub -u mqttuser -P PASSWORD_HERE -t "graywater/dirty/#" -v