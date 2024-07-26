require 'ceedling'

Ceedling.load_project

task :default => %w[ test:all release ]

desc "Build and install headers and linkable objects to PMESDR project."
task :install do

  # Need to figure out how to do cd src; make install
  puts "Installing libutils.a to PMESDR..."
  Dir.chdir('src') do
    sh "make install"
  end

end
