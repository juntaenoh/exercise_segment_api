Pod::Spec.new do |spec|
  spec.name         = "ExerciseSegmentAPI"
  spec.version      = "1.1.0"
  spec.summary      = "Exercise Segment Analysis API for iOS/macOS"
  spec.description  = "A high-performance C API for real-time exercise segment analysis using Google ML Kit pose data."

  spec.homepage     = "https://github.com/juntaenoh/exercise_segment_api"
  spec.license      = { :type => "MIT", :file => "LICENSE" }
  spec.author       = { "Juntae Noh" => "njt9905@naver.com" }

  spec.ios.deployment_target = "12.0"
  spec.osx.deployment_target = "10.15"

  spec.source       = { :git => "https://github.com/juntaenoh/exercise_segment_api.git", :tag => "v#{spec.version}" }

  spec.source_files = "include/*.h", "src/*.c"
  spec.public_header_files = "include/*.h"
  spec.libraries = "m"
  spec.requires_arc = false
  
  # Swift 지원 (선택적)
  spec.swift_version = "5.0"
  
  # MLKit 의존성은 선택적 (사용자가 직접 추가)
  # spec.dependency 'GoogleMLKit/PoseDetection', '~> 4.0'
  
  # 정적 라이브러리 충돌 방지
  spec.static_framework = true
end