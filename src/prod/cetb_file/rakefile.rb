PROJECT_CEEDLING_ROOT = "vendor/ceedling"
load "#{PROJECT_CEEDLING_ROOT}/lib/ceedling.rb"

Ceedling.load_project

task :default => %w[ test:all release ]

desc "Install linkable object."
task :install do
  puts "Installing now."
  from = File.join(PROJECT_BUILD_ROOT,"release","out","c","*.o")
  puts "from #{from}."
end

# Thanks to post at https://groups.google.com/forum/#!topic/throwtheswitch/PbNr1rGzoSk
# for inspiration here
#task :install => [:release]
#task :install, :install_path do |t, args|
#  from = File.join(PROJECT_BUILD_ROOT,"test","out","*.o")
#  to = args[:install_path]
#  FileUtils.cp(Dir.glob(from),to)
#end
