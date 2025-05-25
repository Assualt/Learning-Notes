
use std::collections::HashMap;
use chrono::{DateTime};
use std::os::unix::fs::MetadataExt;
use users;

#[derive(Debug)]
pub struct Args {
    pub short_key : char,
    pub long_key : String,
    pub description : String,
    pub default : String,
    pub need : bool
}

pub struct ArgParse {
    pub key: HashMap<String, Args>,
    pub path_val: String,
    pub parsed_args: HashMap<String, String>
}

pub enum Error {
    InvalidOption
}

impl Error {
    pub fn description(&self) -> String {
        match self {
            Error::InvalidOption => String::from("invalid option")
        }
    }
}

impl ArgParse {
    // 创建一个新的ArgParse实例
    pub fn new() -> ArgParse {
        // 创建一个空的HashMap用于存储key
        ArgParse {
            key: HashMap::new(),
            // 创建一个空的String用于存储path_val
            path_val: String::new(),
            // 创建一个空的HashMap用于存储parsed_args
            parsed_args: HashMap::new()
        }
    }

    // 根据短选项获取对应的参数
    fn _get_short_option(&self, arg_val: &str) -> Result<&Args, String> {
        // 遍历key
        for (_, value) in self.key.iter() {
            // 如果短选项与arg_val的第一个字符相等
            if value.short_key == arg_val.chars().next().unwrap() {
                // 返回对应的参数
                return Ok(value)
            }
        }

        // 如果没有找到对应的参数，返回错误信息
        Err(String::from("invalid option"))
    }


    // 根据传入的参数值，获取对应的选项
    fn _get_long_option(&self, arg_val: &str) -> Result<&Args, String> {
        // 从self.key中获取arg_val对应的值
        match self.key.get(arg_val) {
            // 如果存在，则返回该值
            Some(v) => Ok(v),
            // 如果不存在，则返回错误信息
            None => Err(format!("invalid option: --{}", arg_val))
        }
    }

    // 检查选项是否有效
    fn _check_option(&self, is_long_option: bool, arg_val: &str) -> Result<&Args, String> {
        // 如果不是长选项且选项值长度不为1，则返回错误
        if !is_long_option && arg_val.len() != 1 {
            return Err(format!("invalid option: -{}", arg_val))
        }

        // 如果是长选项，则调用_get_long_option方法获取选项
        if is_long_option {
            return self._get_long_option(arg_val)
        }

        // 如果不是长选项，则调用_get_short_option方法获取选项
        return self._get_short_option(arg_val)
    }

    // 解析参数
    fn parse(&mut self, args: Vec<String>) -> Result<bool, String> {
        // 遍历参数
        for mut idx in 1..args.len() {
            let arg = &args[idx];
            // 如果参数不是以"-"开头，则将其作为路径值
            if !arg.starts_with("-")  {
                self.path_val = arg.to_string();
                break;
            }
            
            // 判断参数是短选项还是长选项
            let index = if arg.starts_with("--") { 2 } else { 1 };
            let long_option = index == 2;
            // 检查选项
            match self._check_option(long_option, &arg[index..]) {
                // 如果选项存在
                Ok(option) => {
                    // 如果选项需要值
                    if option.need {
                        // 如果参数列表中没有下一个参数，则返回错误
                        if idx + 1 >= args.len() {
                            return Err(format!("option:-{}/--{} need value", option.short_key, option.long_key))
                        }
                        // 获取下一个参数
                        idx += 1;
                        let next_arg = &args[idx];
                        // 如果下一个参数不是以"-"开头，则将其作为选项的值
                        if next_arg.starts_with("-") {
                            return Err(format!("option:-{}/--{} need value", option.short_key, option.long_key))
                        }
                        // 将选项和值插入到parsed_args中
                        self.parsed_args.insert(option.long_key.clone(), next_arg.to_string());
                    } else {
                        // 如果选项不需要值，则将选项插入到parsed_args中，值为空字符串
                        self.parsed_args.insert(option.long_key.clone(), String::from(""));
                    }
                },
                // 如果选项不存在，则返回错误
                Err(e) => {
                    return Err(e)
                }
            }
        }
        // 返回成功
        Ok(true)
    }

    fn usage(&self)  {
        println!("usage: ls [options] [path]...");
        println!("options:");
        for (key, value) in self.key.iter() {
            println!("  -{}, --{}\t {}", value.short_key, key, value.description);
        }
    }

    fn _get_file_uid(&self, file_name: &str) -> (String, String, String, u8) {
        let metaval = std::fs::metadata(file_name);
        if metaval.is_err() {
            return (String::from(""), String::from(""), String::from(""), 0)
        }

        let metadata = metaval.unwrap();
        let mut user_name = String::from("root");
        if let Some(user) = users::get_user_by_uid(metadata.uid()) {
            user_name = user.name().to_str().unwrap().to_string();
        }

        let mut group_name = String::from("root");
        if let Some(group) = users::get_group_by_gid(metadata.gid()) {
            group_name = group.name().to_str().unwrap().to_string();
        }

        let hard_link = metadata.nlink() as u8;
        let mode = metadata.mode() & 0o777;
        let pers = [ 'r', 'w', 'x', 'r', 'w', 'x', 'r', 'w', 'x'] ;
        let mut permission = String::new();
        for i in 0..9 {
            if (mode >> (8 - i)) & 0x1 == 1 {
                permission.push_str( &pers[i].to_string());
            } else {
                permission.push_str( &"-".to_string());
            }
        }
        return ( permission, user_name, group_name, hard_link)
    }

    fn _get_file_type(&self, metadata: &std::fs::Metadata) -> String {

        if metadata.is_dir() {
            return String::from("d")
        } else if metadata.is_symlink() {
            return String::from("l")
        }
        return String::from("-")
    }

    fn _show_entry(&self, path: &str, entry: &std::fs::DirEntry, is_show_all: bool, is_show_full: bool, sorted_file: &mut HashMap<String, String>) {
        if !is_show_all && entry.file_name().to_str().unwrap().starts_with(".") {
            return
        }

        if !is_show_full {
            sorted_file.insert(entry.file_name().to_str().unwrap().to_string(), entry.file_name().to_str().unwrap().to_string());
            return
        }

        match entry.metadata() {
            Ok(metadata) => {
                let abs_path = format!("{}/{}", path, entry.file_name().to_str().unwrap());
                let (permission, uid, gid, hard_link) = self._get_file_uid(&abs_path);
                if permission == "" {
                    return
                }
                let file_type = self._get_file_type(&metadata);
                let file_modify_time = metadata.modified().unwrap().duration_since(std::time::UNIX_EPOCH).unwrap().as_secs();
                let datetime = DateTime::from_timestamp(file_modify_time as i64, 0);
                let format_time = datetime.unwrap().format("%m %d %H:%M");

                if metadata.is_symlink() {
                    let link_path = std::fs::read_link(&abs_path).unwrap().to_string_lossy().to_string();
                    let info = format!("{}{}@ {:>4} {:6} {:6} {:>5} {} {} -> {}", file_type, permission, hard_link, uid, gid, metadata.len(), format_time, entry.file_name().to_str().unwrap(), link_path);
                    sorted_file.insert(entry.file_name().to_str().unwrap().to_string(), info);
                } else {
                    let info = format!("{}{}@ {:>4} {:6} {:6} {:>5} {} {}", file_type, permission, hard_link, uid, gid, metadata.len(), format_time, entry.file_name().to_str().unwrap());
                    sorted_file.insert(entry.file_name().to_str().unwrap().to_string(), info);
                }
            },
            Err(error) => {
                println!("access file:{} error e:{}", entry.file_name().to_str().unwrap(), error);
                return
            }
        }
    }

    // 执行命令
    fn exec(&self) {
        // 如果包含help参数，则打印帮助信息并返回
        if self.parsed_args.contains_key("help") {
            self.usage();
            return
        }

        // 如果包含version参数，则打印版本信息并返回
        if self.parsed_args.contains_key("version") {
            println!("ls version 1.0");
            return
        }

        // 获取路径，如果路径为空，则默认为当前目录
        let path = if self.path_val == "" { String::from(".") } else { self.path_val.clone() };
        // 获取是否显示所有文件和目录的参数
        let is_show_all = self.parsed_args.contains_key("all");
        // 获取是否显示完整信息的参数
        let is_show_full = self.parsed_args.contains_key("full");

        // 读取路径下的文件和目录
        match std::fs::read_dir(path.clone()) {
            Ok(entries) => {
                // 创建一个HashMap，用于存储文件和目录的信息
                let mut sorted_file : HashMap<String, String> = HashMap::new();
                // 遍历文件和目录
                for entry in entries {
                    match entry {
                        Ok(file) => {
                            // 调用_show_entry函数，将文件和目录的信息存储到HashMap中
                            self._show_entry(&path, &file, is_show_all, is_show_full, &mut sorted_file);           
                        },
                        Err(error) => {
                            // 打印错误信息并返回
                            println!("access file:{} error e:{}", path, error);
                            return;
                        }
                    }
                }
                // 如果显示完整信息，则打印文件和目录的总数
                if is_show_full {
                    println!("total {}", sorted_file.len());    
                } 

                // 遍历HashMap，打印文件和目录的信息
                let mut idx = 0;
                for (_, value) in sorted_file.iter() {
                    idx += 1;
                    if is_show_full {
                        println!("{}", value);
                    } else {
                        print!("{}\t", value);
                        if idx % 10 == 0 {
                            println!("");
                        }
                    }
                }

                // 如果不显示完整信息，则打印一个空行
                if !is_show_full {
                    println!("");
                }
            },
            Err(error) => {
                // 打印错误信息并返回
                println!("access dir:{} error e:{}", path, error);
                return;
            }
        }
    }

    // 为self添加一个key，key的值为Args类型
    fn add<T>(&mut self, short_key: char, long_key: &str, description: &str, need_val : bool, _: T) {
        // 如果long_key为空，则panic
        if long_key == "" {
            panic!("long key can't be empty");
        }
        
        // 将long_key转换为字符串，并插入到self.key中，值为Args类型
        self.key.insert(long_key.to_string(), Args {
            short_key,
            long_key: String::from(long_key),
            description: String::from(description),
            default: String::from(""),
            need: need_val
        });
    }
}

fn main() {
    // 获取命令行参数
    let args = std::env::args();
    // 创建一个ArgParse对象
    let mut arg = ArgParse::new();

    // 添加-h参数，用于显示帮助信息
    arg.add('h', "help", "show this help message", false, "");
    // 添加-a参数，用于显示所有文件，包括以.开头的文件
    arg.add('a', "all", "show all files, including files starting with .", false, "");
    // 添加-l参数，用于显示完整路径
    arg.add('l', "full", "show  .", false, "");
    // 添加-v参数，用于显示版本信息
    arg.add('v', "version", "show version for this tool", false, "");

    // 解析命令行参数
    match arg.parse(Vec::from_iter(args)) {
        // 如果解析成功，执行arg.exec()方法
        Ok(_) => {
            arg.exec()
        },
        // 如果解析失败，打印错误信息，并执行arg.usage()方法
        Err(e) => {
            println!("{}", e);
            arg.usage();
        }
    }
}
