require 'xcodeproj'
project_path = './build/binding.xcodeproj'
project = Xcodeproj::Project.open(project_path)
# puts project.schemes.inspect
schemes1 = Xcodeproj::Project.schemes('./build/binding.xcodeproj/xcshareddata/xcschemes/fsevents.xcscheme')
puts schemes1.inspect

schemes2 = Xcodeproj::XCScheme.new('/Users/robertblackwell/MyJSPackages/fsevents/build/binding.xcodeproj/xcshareddata/xcschemes/fsevents.xcscheme')
puts schemes2.launch_action.command_line_arguments.to_s
schemes2.launch_action.command_line_arguments =  Xcodeproj::XCScheme::CommandLineArguments.new([{ :argument => '${SRCROOT}/test/108_array.js', :enabled => true }])#Xcodeproj::XCScheme::CommandLineArguments.new({'argument' => 'one', 'enabled' => true})
puts schemes2.launch_action.command_line_arguments.to_s
