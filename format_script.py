import os
import argparse
import subprocess
import sys

def find_clang_format():
    """查找系统中的clang-format可执行文件"""
    # 尝试常见的clang-format命令
    return "D:\\Qt\\Tools\\llvm-mingw1706_64\\bin\\clang-format.exe"

def find_cpp_files(root_dir):
    """递归查找根目录下所有的C++文件"""
    cpp_extensions = ['.cpp', '.h', '.hpp', '.cc', '.cxx', '.c++', '.hh', '.hxx']
    cpp_files = []
    
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if any(filename.endswith(ext) for ext in cpp_extensions):
                cpp_files.append(os.path.abspath(os.path.join(dirpath, filename)))
    
    return cpp_files

def format_files(clang_format_cmd, config_file, files, dry_run=False):
    """使用clang-format格式化文件"""
    if not files:
        print("没有找到C++文件需要格式化")
        return
    
    print(f"找到 {len(files)} 个C++文件")
    print(f"使用配置文件: {config_file}")
    
    success_count = 0
    fail_count = 0
    failed_files = []
    
    # 构建格式化命令
    for file_path in files:
        try:
            if "\\debug\\" in file_path:
                continue
            print(f"正在处理: {file_path}")
            
            cmd = [
                clang_format_cmd,
                f'--style=file:{config_file}',
                '--verbose'
            ]
            
            if not dry_run:
                cmd.append('-i')  # 原地修改文件
            
            cmd.append(file_path)
            
            # 执行格式化命令
            result = subprocess.run(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True
            )
            
            if result.returncode != 0:
                print(f"格式化失败: {file_path}, 错误: {result.stderr}")
                fail_count += 1
                failed_files.append(file_path)
            else:
                success_count += 1
                
        except Exception as e:
            print(f"处理文件时出错 {file_path}: {str(e)}")
            fail_count += 1
            failed_files.append(file_path)
    
    print("\n格式化完成:")
    print(f"成功: {success_count} 个文件")
    print(f"失败: {fail_count} 个文件")
    
    if failed_files:
        print("\n失败的文件列表:")
        for file in failed_files:
            print(f"  - {file}")

def main():
    # 解析命令行参数
    parser = argparse.ArgumentParser(description='使用clang-format格式化所有C++文件')
    parser.add_argument('--dir', default='.', help='要搜索C++文件的根目录，默认为当前目录')
    parser.add_argument('--config', default='.clang-format', help='clang-format配置文件路径，默认为当前目录下的.clang-format')
    parser.add_argument('--dry-run', action='store_true', help='只显示要格式化的文件，不实际修改')
    
    args = parser.parse_args()
    
    # 检查根目录是否存在
    root_dir = os.path.abspath(args.dir)
    if not os.path.isdir(root_dir):
        print(f"错误: 目录 '{root_dir}' 不存在", file=sys.stderr)
        sys.exit(1)
    
    # 检查配置文件是否存在
    config_file = os.path.abspath(args.config)
    if not os.path.isfile(config_file):
        print(f"错误: 配置文件 '{config_file}' 不存在", file=sys.stderr)
        sys.exit(1)
    
    # 查找clang-format
    clang_format_cmd = find_clang_format()
    if not clang_format_cmd:
        print("错误: 未找到clang-format，请确保已安装clang-format", file=sys.stderr)
        sys.exit(1)
    
    # 查找所有C++文件
    cpp_files = find_cpp_files(root_dir)
    
    # 格式化文件
    format_files(clang_format_cmd, config_file, cpp_files, args.dry_run)

if __name__ == "__main__":
    main()
    