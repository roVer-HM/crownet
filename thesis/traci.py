import socket


def main():
    # Create a socket object
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    host = '0.0.0.0'
    port = 9998
    print(f"Socket created on {host}:{port}")
    # Bind the socket to the address and port
    server_socket.bind((host, port))

    # Listen for incoming connections
    server_socket.listen(1)
    print(f"Listening on {host}:{port}...")

    # Accept and handle incoming connections
    while True:
        client_socket, addr = server_socket.accept()
        print(f"Connection from {addr}")

        # Receive and print messages
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            print(f"Message from {addr}: {data.decode('utf-8')}")

        # Close the client socket
        client_socket.close()
        print(f"Connection closed with {addr}")


if __name__ == '__main__':
    main()
