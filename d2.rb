#!/usr/bin/env ruby

require 'rubygems'
require 'xcodeproj'

puts "Project path:"
proj_path = gets
puts "New target name:"
name = gets
puts "New target bundle identifer"
bundleIdentifier = gets
puts "Witch target to clone?"
srcTargetName =  gets

name = name.chomp
bundleIdentifier = bundleIdentifier.chomp
proj_path = proj_path.chomp
srcTargetName = srcTargetName.chomp

proj = Xcodeproj::Project.open(proj_path)
src_target = proj.targets.find { |item| item.to_s == srcTargetName }
#proj_path = "/testingTest.xcodeproj"

# create target
target = proj.new_target(src_target.symbol_type, name, src_target.platform_name, src_target.deployment_target)
target.product_name = name


# create scheme
scheme = Xcodeproj::XCScheme.new
scheme.add_build_target(target)
scheme.set_launch_target(target)
scheme.save_as(proj_path, name, shared = true)

# copy build_configurations
target.build_configurations.map do |item|
  item.build_settings.update(src_target.build_settings(item.name))
end

target.build_configurations.each do |config|
  config.build_settings['PRODUCT_BUNDLE_IDENTIFIER'] = bundleIdentifier
  config.build_settings['PRODUCT_NAME'] = "$(TARGET_NAME)"
end
