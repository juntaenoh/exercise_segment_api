Pod::Spec.new do |spec|
  spec.name         = "ExerciseSegmentAPI"
  spec.version      = "2.0.0"
  spec.summary      = "Exercise Segment Analysis API v2.0.0"
  spec.description  = <<-DESC
                     A C API for analyzing exercise progress and providing real-time feedback based on Google ML Kit pose data.
                     
                     Features:
                     - 33 landmark pose analysis
                     - Real-time progress tracking
                     - JSON-based workout storage
                     - Calibration system
                     - Swift integration support
                     DESC

  spec.homepage     = "https://github.com/your-org/exercise_segment_api"
  spec.license      = { :type => "MIT", :file => "LICENSE" }
  spec.author       = { "Your Name" => "your.email@example.com" }

  spec.platform     = :ios, "16.0"
  spec.source       = { :path => "." }

  spec.source_files = [
    "include/**/*.h",
    "src/**/*.c"
  ]

  spec.public_header_files = "include/**/*.h"
  
  spec.requires_arc = false
  
  spec.pod_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/include'
  }
  
  spec.user_target_xcconfig = {
    'HEADER_SEARCH_PATHS' => '$(PODS_TARGET_SRCROOT)/include'
  }
end
