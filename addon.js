/*
** © 2018 by Philipp Dunkel, Ben Noordhuis, Elan Shankar
** Licensed under MIT License.
*/

/* jshint node:true */
'use strict';
const util = require('util')
if (process.platform !== 'darwin') {
	throw new Error(`Module 'fsevents' is not compatible with platform '${process.platform}'`);
}

const Native = require('./fsevents.node');
/**
* Watch one or more directories and all files and subdirectories
* @param {string| string[]} paths An array of one or more directories or a string representing a single directory.
* @param {(path : string, flags:number, eventInfo:number) => void} Handler to receive directory events
* @return {()=>void} A function to call to stop the watcher created by this call
*/
function watch(path, handler) {
	// if ('object' !== typeof path && Array.isArray(path)) throw new TypeError(`argument 1 must be an array and not a ${typeof handler}`);
	// if ('function' !== typeof handler) throw new TypeError(`argument 2 must be a function and not a ${typeof handler}`);

	let instance = Native.start(path, handler);
	console.log("Instance : ", util.inspect(Native.start), util.inspect(Native.stop))
	if (!instance) 
		throw new Error(`could not watch: ${path}`);
	return () => {
		const result = instance ? Promise.resolve(instance).then(Native.stop) : null;
		instance = null;
		return result;
	};
}
function getInfo(path, flags) {
	return {
		path, flags,
		event: getEventType(flags),
		type: getFileType(flags),
		changes: getFileChanges(flags)
	};
}

function getFileType(flags) {
	if (con.kFSEventStreamEventFlagItemIsFile & flags) return 'file';
	if (con.kFSEventStreamEventFlagItemIsDir & flags) return 'directory';
	if (con.kFSEventStreamEventFlagItemIsSymlink & flags) return 'symlink';
}
function getEventType(flags) {
	if (con.kFSEventStreamEventFlagItemRemoved & flags) return 'deleted';
	if (con.kFSEventStreamEventFlagItemRenamed & flags) return 'moved';
	if (con.kFSEventStreamEventFlagItemCreated & flags) return 'created';
	if (con.kFSEventStreamEventFlagItemModified & flags) return 'modified';
	if (con.kFSEventStreamEventFlagRootChanged & flags) return 'root-changed';

	return 'unknown';
}
function getFileChanges(flags) {
	return {
		inode: !!(con.kFSEventStreamEventFlagItemInodeMetaMod & flags),
		finder: !!(con.kFSEventStreamEventFlagItemFinderInfoMod & flags),
		access: !!(con.kFSEventStreamEventFlagItemChangeOwner & flags),
		xattrs: !!(con.kFSEventStreamEventFlagItemXattrMod & flags)
	};
}

exports.watch = watch;
exports.getInfo = getInfo;
exports.constants = con;
exports.func = func;
