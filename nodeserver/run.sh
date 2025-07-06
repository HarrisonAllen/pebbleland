docker stop node-server
docker rm node-server
docker run -d -p 5001:5001 --mount type=bind,src=./database,dst=/app/prisma/database --name node-server node-server 