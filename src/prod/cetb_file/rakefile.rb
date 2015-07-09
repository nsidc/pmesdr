PROJECT_CEEDLING_ROOT = "vendor/ceedling"
load "#{PROJECT_CEEDLING_ROOT}/lib/ceedling.rb"

Ceedling.load_project

task :default => %w[ test:all release ]

desc "Install headers and linkable objects to PMESDR project."
task :install do

  # Need to figure out how to make the include and lib dirs if
  # they don't already exist
  puts "Installing .o files:"
  from = File.join(PROJECT_BUILD_ROOT,"release","out","c","*.o")
  to = "#{ENVIRONMENT_PMESDR_TOP_DIR}/lib"
  puts "from #{from}"
  puts "to   #{to}"
  FileUtils.cp(Dir.glob(from),to)

  # I'm sure there's a better way to do this,
  # (there has to be a way to access the list of file you get
  # when you run rake files:header)
  # but for now:
  puts "Installing .h files:"
  from = File.join(PROJECT_BUILD_ROOT,"..","src","*.h")
  to = "#{ENVIRONMENT_PMESDR_TOP_DIR}/include"
  puts "from #{from}"
  puts "to   #{to}"
  FileUtils.cp(Dir.glob(from),to)

end

# Thanks to post at https://groups.google.com/forum/#!topic/throwtheswitch/PbNr1rGzoSk
# for inspiration here
#task :install => [:release]
#task :install, :install_path do |t, args|
#  from = File.join(PROJECT_BUILD_ROOT,"test","out","*.o")
#  to = args[:install_path]
#  FileUtils.cp(Dir.glob(from),to)
#end
