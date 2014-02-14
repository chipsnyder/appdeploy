# encoding: utf-8

require 'rubygems'
require 'rake'

task :default => 'compile'

desc 'Compile appdeploy'
file 'compile' => ['appdeploy.c'] do |t|
  system %Q[gcc -Wall -o "appdeploy" -framework CoreFoundation -framework MobileDevice -F/System/Library/PrivateFrameworks "#{t.prerequisites.join('" "')}"]
end

desc 'Install appdeploy on the system'
task :install => 'appdeploy' do |t|
  system %Q[/bin/cp -f "#{t.prerequisites.join('" "')}" /usr/local/bin/]
end

desc 'Uninstall appdeploy from the system'
task :uninstall do 
  system 'rm -f /usr/local/bin/appdeploy'
end

desc 'Cleanup'
task :clean do
  system 'rm -f appdeploy'
end

desc 'Update appdeploy (make sure you get latest from github first)'
task :update => [:compile, :uninstall, :setup]

desc 'Setup appdeploy (make sure you get latest from github first)'
task :setup => [:compile, :install, :clean]
