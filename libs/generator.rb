#
#
#
require 'yaml'

def list_dirs(dir = '.')
    return [] unless File.directory?(dir)
    list = Dir.children(dir).collect do |d|
        path = File.join(dir, d)
        File.directory?(path) ? path : nil
    end
    list.compact
end

def generate_file(dir_path, c_outfile, rb_outfile)
    conf_file = File.join(dir_path, 'conf.yaml')
    return unless File.file?(conf_file)
    conf = YAML.load_file(conf_file)

    rb_outfile.puts
    rb_outfile.puts "# Library: #{conf['name'] || 'unknown'}"
    rb_outfile.puts "# Description: #{conf['description'] || 'No description provided.'}"
    rb_outfile.puts "# Version: #{conf['version'] || '-'}"
    rb_outfile.puts

    # Generate Ruby code
    ruby_files = conf["ruby-files"]
    return if ruby_files.nil?
    if ruby_files.is_a?(String)
        ruby_files = [ruby_files]
    end
    ruby_files.each do |rbfile|
        rb_outfile.puts File.read(File.join(dir_path, rbfile))
    end

    # generate C code

end

if __FILE__ == $0
    dir = ARGV[0] || '.'
    c_outfile = STDOUT;
    rb_outfile = STDOUT;
    # c header
    c_outfile.puts "/* ============================="
    c_outfile.puts " * Generated Library Information"
    c_outfile.puts " * This file is auto-generated."
    c_outfile.puts " * Your code will be overwitten."
    c_outfile.puts " *    Do not edit manually."
    c_outfile.puts " * ============================= */"
    c_outfile.puts
    # ruby header
    rb_outfile.puts "# ============================="
    rb_outfile.puts "# Generated Library Information" 
    rb_outfile.puts "# This file is auto-generated."
    rb_outfile.puts "# Your code will be overwitten."
    rb_outfile.puts "#    Do not edit manually."
    rb_outfile.puts "# ============================="
    rb_outfile.puts
    # library list
    list_dirs(dir).each do |d|
        generate_file(d, c_outfile, rb_outfile)
    end
end

