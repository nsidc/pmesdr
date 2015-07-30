require 'ceedling'

Ceedling.load_project

task :default => %w[ test:all release ]

desc "Build and install headers and linkable objects to PMESDR project."
task :install do

  # Need to figure out how to do cd src; make install
  puts "Installing libcetb.a to PMESDR..."
  Dir.chdir('src') do
    sh "make install"
  end

end

# Thanks to post at https://groups.google.com/forum/#!topic/throwtheswitch/PbNr1rGzoSk
# for inspiration here
#task :install => [:release]
#task :install, :install_path do |t, args|
#  from = File.join(PROJECT_BUILD_ROOT,"test","out","*.o")
#  to = args[:install_path]
#  FileUtils.cp(Dir.glob(from),to)
#end
