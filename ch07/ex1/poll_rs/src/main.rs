use std::str;
use std::time::Duration;
use tokio::io::AsyncReadExt;
use tokio::time;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + 'static>> {
    let mut fd = tokio::io::stdin();
    let mut buf = [0; 1024];

    let fut = fd.read(&mut buf);
    let v = match time::timeout(Duration::from_secs(2), fut).await {
        Ok(v) => v,
        Err(e) => {
            eprintln!("{}", e);
            return Ok(());
        }
    };

    println!(
        "Bytes read: {}, string: {}",
        v.expect("failed to read"),
        str::from_utf8(&buf).expect("doesn't look like utf-8")
    );

    // The output will need an additional return to break out of the tokio executor as described in
    // the documentation. https://docs.rs/tokio/latest/tokio/io/fn.stdin.html
    Ok(())
}
