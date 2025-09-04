Pod::Spec.new do |spec|
  spec.name         = "ExerciseSegmentAPI"
  spec.version      = "1.0.0"
  spec.summary      = "Exercise Segment Analysis API for iOS/macOS"
  spec.description  = <<-DESC
    A high-performance C API for real-time exercise segment analysis using Google ML Kit pose data.
    Provides joint-specific 3D vector feedback for exercise correction with 60fps real-time processing.
  DESC

  spec.homepage     = "https://github.com/your-username/exercise_segment_api"
  spec.license      = { :type => "MIT", :file => "LICENSE" }
  spec.author       = { "Your Name" => "your.email@example.com" }

  # Platform support
  spec.ios.deployment_target = "12.0"
  spec.osx.deployment_target = "10.15"
  spec.tvos.deployment_target = "12.0"
  spec.watchos.deployment_target = "5.0"

  # Source
  spec.source       = { :git => "https://github.com/your-username/exercise_segment_api.git", :tag => "#{spec.version}" }

  # Source files
  spec.source_files = "include/*.h", "src/*.c"
  spec.public_header_files = "include/*.h"

  # Dependencies
  spec.libraries = "m"

  # Compiler flags
  spec.compiler_flags = "-O3", "-ffast-math"

  # Module map for Swift interoperability
  spec.module_map = "include/ExerciseSegmentAPI.modulemap"

  # Documentation
  spec.documentation_url = "https://github.com/your-username/exercise_segment_api/blob/main/README.md"

  # Social media
  spec.social_media_url = "https://twitter.com/your_username"

  # Keywords for CocoaPods search
  spec.keywords = "exercise", "pose", "analysis", "mlkit", "fitness", "realtime", "3d", "motion"

  # Requires ARC
  spec.requires_arc = false

  # C standard
  spec.c_standard = "c99"
end
