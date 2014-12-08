#!/usr/bin/env ruby

require 'github/markup'

input = 'README.md'
output = 'README.html'
File.write(output, GitHub::Markup.render(input, File.read(input)))

