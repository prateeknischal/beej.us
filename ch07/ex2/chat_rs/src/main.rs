use std::collections::HashMap;
use std::io::prelude::*;
use std::net::{SocketAddr, TcpListener, TcpStream};
use std::sync::mpsc;
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    let listener = TcpListener::bind("127.0.0.1:9000").unwrap();

    // A channel that lives throughout the server lifetime
    let (rx, tx) = mpsc::channel::<(SocketAddr, String)>();

    // The connection map to keep track of the connection tuple and a clone of the stream. The clone
    // of the stream will be used to broadcast the messages. When using tokio, we could just use
    // tokio::io::split() to get the readable and writable halves.
    let conns = Arc::new(Mutex::new(HashMap::<SocketAddr, TcpStream>::new()));

    let broadcast_conn_map = conns.clone();
    thread::spawn(move || loop {
        for (from_addr, msg) in tx.iter() {
            broadcast_conn_map.lock().unwrap().iter().for_each(|kv| {
                let (addr, mut stream) = kv;
                if addr.eq(&from_addr) {
                    return;
                }

                let _ = stream.write(format!("[{}] ", addr).as_bytes());
                let _ = stream.write(msg.as_bytes()).unwrap();
            });
        }
    });

    loop {
        let (mut stream, remote_addr) = listener.accept().unwrap();
        eprintln!("Got a connection: {}", remote_addr);

        // Add the new connection to the connection map to broadcast the messages from other
        // messages.
        conns
            .lock()
            .unwrap()
            .insert(remote_addr, stream.try_clone().unwrap());

        // The sender copy of the channel for the client to own and send their messages.
        let client_sender = rx.clone();

        // client copy of the connection map to remove the remote_addr from the map once the
        // connection is closed.
        let conn_closer_map = conns.clone();
        thread::spawn(move || {
            let mut buf = [0u8; 1024];
            loop {
                match stream.read(&mut buf) {
                    Ok(0) => {
                        break;
                    }
                    Ok(n) => {
                        let out = std::str::from_utf8(&buf[0..n]).unwrap().to_string();
                        eprint!("[{}] {}", remote_addr, &out);

                        let _ = client_sender.send((remote_addr, out));

                        buf[0..n].iter_mut().for_each(|v| *v = 0);
                    }
                    Err(e) => {
                        eprintln!("[{}] {}", remote_addr, e);
                        break;
                    }
                }
            }

            // Remove the connection from the connection map.
            conn_closer_map.lock().unwrap().remove(&remote_addr);
        });
    }
}
