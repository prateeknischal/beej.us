use std::net::TcpStream;

fn main() {
    let stream = match TcpStream::connect("www.example.com:80") {
        Ok(s) => s,
        Err(e) => panic!("{}", e),
    };

    match stream.peer_addr() {
        Ok(a) => {
            println!("IP Address : {}", a.ip());
            println!("Port Number: {}", a.port());
        }
        Err(e) => {
            panic!("{}", e);
        }
    }
}
