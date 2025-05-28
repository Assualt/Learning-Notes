
use argparse::{ArgumentParser, ArgumentParserTrait};

fn main() {
    println!("Hello, world!");
    
    let mut parser = ArgumentParser::new("test_prog", "test description", "1.0");
    
    parser.add_argument(Some('h'), "help", "show help", false, "");
    parser.add_argument(Some('v'), "version", "show version", false, "");
    parser.add_argument(Some('f'), "file", "file to read", true, "cerfile.txt");
    parser.add_argument(Some('c'), "cert", "file to read", true, 123);
    
    parser.show_help();
    
    let args = vec![ 
        String::from("test_prog"), 
        String::from("-v"), 
        String::from("-f"), String::from("certfile.txt"),
        String::from("-c"), String::from("456"),
        String::from("-h")
    ];
    // let args = std::env::args();
    match parser.parse(args) {
        Ok(_) => {
            let file_arg = parser.get_arg::<String>("file");
            let cert_arg = parser.get_arg::<u32>("cert");
            println!("file: {}, cert: {}", file_arg == None, cert_arg == None);
            
            let version = parser.get_arg::<String>("version");
            println!("version: {}", version == None);
        },
        Err(_) => {
            println!("argument parse error\n{}", parser.get_error().unwrap());
        }
    };
}
