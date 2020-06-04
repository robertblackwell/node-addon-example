#!/usr/bin/env ruby

require 'rubygems'
require 'xcodeproj'

name = 'test_copy'

proj = Xcodeproj::Project.open('build/binding.xcodeproj')
src_target = proj.targets.find { |item| item.to_s == 'test' }

# create target
target = proj.new_target(src_target.symbol_type, name, src_target.platform_name, src_target.deployment_target)
target.product_name = name

# create scheme
scheme = Xcodeproj::XCScheme.new
scheme.add_build_target(target)
scheme.set_launch_target(target)
scheme.save_as(proj_path, name)

# copy build_configurations
target.build_configurations.map do |item|
  item.build_settings.update(src_target.build_settings(item.name))
end

# copy build_phases
phases = src_target.build_phases.reject { |x| x.instance_of? Xcodeproj::Project::Object::PBXShellScriptBuildPhase }.collect(&:class)

phases.each do |klass|
  src = src_target.build_phases.find { |x| x.instance_of? klass }
  dst = target.build_phases.find { |x| x.instance_of? klass }
  unless dst
    dst ||= proj.new(klass)
    target.build_phases << dst
  end
  dst.files.map { |x| x.remove_from_project }

  src.files.each do |f|
    file_ref = proj.new(Xcodeproj::Project::Object::PBXFileReference)
    file_ref.name = f.file_ref.name
    file_ref.path = f.file_ref.path
    file_ref.source_tree = f.file_ref.source_tree
    file_ref.last_known_file_type = f.file_ref.last_known_file_type
    file_ref.fileEncoding = f.file_ref.fileEncoding
    begin
      file_ref.move(f.file_ref.parent)
    rescue
    end

    build_file = proj.new(Xcodeproj::Project::Object::PBXBuildFile)
    build_file.file_ref = f.file_ref
    dst.files << build_file
  end
end

# add files
classes = proj.main_group.groups.find { |x| x.to_s == 'Group' }.groups.find { |x| x.name == 'Classes' }
sources = target.build_phases.find { |x| x.instance_of? Xcodeproj::Project::Object::PBXSourcesBuildPhase }
file_ref = classes.new_file('test.m')
build_file = proj.new(Xcodeproj::Project::Object::PBXBuildFile)
build_file.file_ref = file_ref
sources.files << build_file

proj.save