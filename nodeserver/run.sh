docker run -d -p 5001:5001 --mount type=bind,src=./database,dst=/app/database --name node-server node-server 