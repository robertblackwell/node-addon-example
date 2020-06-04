require 'xcodeproj'
project_path = 'build-copy/binding.xcodeproj'
project = Xcodeproj::Project.open(project_path)

project.targets.each do |target|
  puts target.name
end