use std::any::{Any, TypeId};
use std::collections::HashMap;
use std::fmt::{Display};

#[derive(Debug)]
pub enum ArgumentError {
    MissingArgument = 0,
    InvalidArgument = 1,
    RangeError = 2,
    InvalidLongKey = 3,
    DuplicateLongKey = 4,
    NeedValue =5,
}

impl Display for ArgumentError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", "")
    }
}

#[derive(Debug)]
pub struct Variant(Box<dyn Any>);

impl Variant {
    pub fn new<T: Any>(value: T) -> Self {
        Variant(Box::new(value))
    }
    
    fn dynamic_downcast<T: Any>(&self) -> Option<&T> {
        self.0.downcast_ref::<T>()
    }
}

impl Display for Variant {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self)
    }
}

pub struct ArgumentOption {
    pub short_key: Option<char>,
    pub long_key: String,
    pub description: String,
    pub required: bool,
    pub default: Variant,
    pub type_id: TypeId
}

pub trait ArgumentParserTrait {
    fn new(prog_name: &str, desc: &str, version: &str) -> Self;
    fn add_argument<T: 'static>(&mut self, short_key: Option<char>, long_key: &str, desc: &str, required: bool, default: T);
    fn parse(&mut self, args: Vec<String>) -> Result<(), ArgumentError>;
    fn show_help(&self);
    fn check_option(&self, key: &str) -> (bool, String);
    fn get_arg<T>(&self, key: &str) -> Option<&T> where T : Any;
    fn get_error(&self) -> Option<String>;
}

pub struct ArgumentParser {
    pub prog_name: String,
    pub prog_description: String,
    pub options: HashMap<String, ArgumentOption>,
    pub parsed_options: HashMap<String, Variant>,
    pub arg_path: String,
    pub version: String,
}

impl ArgumentParserTrait for ArgumentParser {
    fn new(prog_name: &str, desc: &str, version: &str) -> Self {
        ArgumentParser {
            prog_name: String::from(prog_name),
            prog_description: String::from(desc),
            options: HashMap::new(),
            parsed_options: HashMap::new(),
            arg_path: String::from(""),
            version: String::from(version),
        }
    }

    fn add_argument<T: 'static>(
        &mut self,
        short_key: Option<char>,
        long_key: &str,
        desc: &str,
        required: bool,
        default: T,
    ) {
        if long_key.len() <= 1 {
            panic!("long key must be more than 1 character. {}", long_key);
        }

        if self.options.contains_key(long_key) {
            panic!("long key already exists. {}", long_key);
        }

        let option = ArgumentOption {
            short_key,
            long_key: String::from(long_key),
            description: String::from(desc),
            required,
            default: Variant::new(default),
            type_id: TypeId::of::<T>()
        };
        self.options.insert(long_key.to_string(), option);
    }

    fn parse(&mut self, args: Vec<String>) -> Result<(), ArgumentError> {
        let mut index = 1;
        while index < args.len() {
            let arg = args[index].clone();
            if !arg.starts_with("-") && !arg.starts_with("--") {
                self.arg_path = String::from(arg);
                break;
            }

            let option_key = if arg.starts_with("--") {
                &arg[2..]
            } else {
                &arg[1..]
            };

            let (ok, long_key) = self.check_option(option_key);
            if !ok {
                return Err(ArgumentError::InvalidArgument);
            }

            let option = self.options.get_mut(&long_key).unwrap();
            if option.required {
                if index + 1 >= args.len() {
                    // mark as default
                    return Err(ArgumentError::NeedValue);
                }
                
                let next_arg = args[index + 1].clone();
                // !todo()
                
                // convert next_arg to the type of the option
                self.parsed_options.insert(long_key, Variant::new(next_arg));
                index += 1;
            } else {
                self.parsed_options.insert(long_key, Variant::new(()));
            }

            index += 1;
        }

        Ok(())
    }
    
    fn show_help(&self) {
        println!("usage: {} [options] [path]", self.prog_name);
        println!("desc :{}", self.prog_description);
        println!("options:");
        for (key, value) in self.options.iter() {
            if let Some(short_key) = value.short_key {
                println!("  -{}/--{}\t {}", short_key, key, value.description);
            } else {
                println!("     --{}\t {}", key, value.description);
            }
        }
    }

    fn check_option(&self, key: &str) -> (bool, String) {
        let mut long_key = String::from(key);
        if key.len() == 1 {
            self.options.iter().for_each(|(k, v)| {
                if v.short_key == Some(key.chars().next().unwrap()) {
                    long_key = k.clone();
                }
            });
        }

        (self.options.contains_key(&long_key), long_key)
    }
    
    fn get_arg<T>(&self, key: &str) -> Option<&T> where T: Any {
        let (ok, _) = self.check_option(key);
        if !ok {
            return None;
        }
        
        if let Some(value) = self.parsed_options.get(key) {
            return value.dynamic_downcast::<T>();
        }
        
        let default_value = &self.options.get(key)?.default;
        default_value.dynamic_downcast()
    }

    fn get_error(&self) -> Option<String> {
        for (key, value) in self.options.iter() {
            if value.required && !self.parsed_options.contains_key(key) {
                return Some(format!("argument --{} is required", key));
            }
        }
        
        None
    }
}
