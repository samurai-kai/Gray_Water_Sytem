@REM just run .\start_port.bat to start the service
@REM and ensure docker engine is running

docker compose pull
docker compose up -d


@REM if port fail
docker compose down
docker compose up -d
