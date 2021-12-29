use std::net::SocketAddr;
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpListener;
use tokio::net::TcpStream;
use tokio::time::{sleep, Duration};

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let listener = match TcpListener::bind("0.0.0.0:9000").await {
        Ok(s) => s,
        Err(e) => panic!("{}", e),
    };

    loop {
        let (socket, remote_addr) = listener.accept().await?;
        tokio::spawn(async move {
            handle_conn(socket, remote_addr).await;
        });
    }
}

async fn handle_conn(mut stream: TcpStream, remote_addr: SocketAddr) {
    let mut buf = [0u8; 1 << 12];

    loop {
        let bytes_read: usize = match stream.read(&mut buf).await {
            Ok(bytes_read) => {
                println!("[{}] bytes read: {}", remote_addr, bytes_read);
                bytes_read
            }
            Err(e) => {
                eprintln!("[{}] failed to read: {}", remote_addr, e);
                0
            }
        };

        if bytes_read == 0 {
            return;
        }

        // wait a little before writing on the connection
        sleep(Duration::from_millis(500)).await;

        if let Err(e) = stream.write_all(&buf[0..bytes_read]).await {
            eprintln!("[{}] failed to write bytes: {}", remote_addr, e);
            return;
        };

        println!("[{}] closing connection", remote_addr);
    }
}
