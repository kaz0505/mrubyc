#!/usr/bin/env ruby
#
# library generator
#   integrates C source files into mruby/c library.
#   library files should be placed in "mrbc_gems" folder.
#
#  Copyright (C) 2015- Kyushu Institute of Technology.
#  Copyright (C) 2015- Shimane IT Open-Innovation Center.
#
#  This file is distributed under BSD 3-Clause License.
#
# (usage)
# ruby gen_lib.rb mrubyc_folder
#

if !ARGV[0]
  puts <<EOL
(usage)
ruby gen_lib.rb mrubyc_folder
EOL
  exit 1
end
mrubyc_folder = ARGV[0]


