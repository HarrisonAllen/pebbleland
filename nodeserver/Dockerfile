# Use the Node.js image as the base image
FROM node:22-bookworm-slim

# RUN mkdir -p /app && chown -R node:node /app


# Set the working directory inside the container
WORKDIR /app

# Copy package.json and package-lock.json to the working directory
COPY package.json package-lock.json ./

# Install the project dependencies
RUN npm install

# Copy the entire application files to the working directory
COPY . .

# Build the ts app
RUN npm run build

# Prisma time
RUN apt-get update -y && apt-get install -y openssl
RUN npx prisma migrate dev --name initialize
# RUN npx prisma db push
RUN npx prisma generate

# Expose port 5001 for the WebSocket server
EXPOSE 5001


# Command to run the WebSocket server when the container starts
CMD ["node", "dist/app.js"]