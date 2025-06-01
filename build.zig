const std = @import("std");

//fn collect_c_files(allocator: *std.mem.Allocator, directoryPath: []const u8) !std.ArrayList([]const u8) {
//    var ans = std.ArrayList([]const u8).init(allocator);
//}

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "test",
        .target = target,
        .optimize = optimize,
    });

    // Tell Zig to treat this as a C++ source file
    exe.addCSourceFiles(.{
        .files = &.{"src/main.cpp"},
        .flags = &.{
            "-std=c++20", // C++ standard
            "-Wall", // Enable all warnings
            "-Wextra", // Extra warnings
        },
    });

    // Optional: link against libc++ instead of default
    exe.linkSystemLibrary("c++");
    exe.linkLibC();

    // Install step
    b.installArtifact(exe);
}
