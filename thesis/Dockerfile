# Use an official Python runtime as a parent image
FROM python:3.9-slim

# Set the working directory to /app
WORKDIR /app

# Copy the server.py file into the container at /app
COPY traci.py /app

# Make port 9998 available to the world outside this container
EXPOSE 9998

# Define environment variable
ENV NAME World

# Run server.py when the container launches
CMD ["python", "traci.py"]
