# ExerciseSegmentAPI Swift 사용법

## 개요

ExerciseSegmentAPI는 Swift에서 GoogleMLKit과 함께 사용할 수 있도록 최적화된 운동 세그먼트 분석 라이브러리입니다.

## 설치

### CocoaPods

```ruby
pod 'ExerciseSegmentAPI', '~> 1.0'
```

### 의존성

- GoogleMLKit/PoseDetection ~> 4.0
- iOS 12.0+
- Swift 5.0+

## 기본 사용법

### 1. 초기화

```swift
import ExerciseSegmentAPI
import MLKit

let segmentManager = ExerciseSegmentManager()

do {
    try segmentManager.initialize()
    print("API 초기화 성공")
} catch {
    print("초기화 실패: \(error)")
}
```

### 2. 캘리브레이션

```swift
// GoogleMLKit에서 포즈 감지
let poseDetector = PoseDetector.poseDetector(options: PoseDetectorOptions())

// 기본 포즈로 캘리브레이션
do {
    try segmentManager.calibrate(with: basePose)
    print("캘리브레이션 완료")
} catch {
    print("캘리브레이션 실패: \(error)")
}
```

### 3. 세그먼트 생성

```swift
// 시작 키포즈와 종료 키포즈 정의
let startKeypose: Pose = // ... 시작 자세
let endKeypose: Pose = // ... 종료 자세

// 관심 관절들 정의
let careJoints: [JointType] = [
    .leftShoulder, .rightShoulder,
    .leftElbow, .rightElbow,
    .leftWrist, .rightWrist,
    .leftHip, .rightHip
]

do {
    try segmentManager.createSegment(
        startKeypose: startKeypose,
        endKeypose: endKeypose,
        careJoints: careJoints
    )
    print("세그먼트 생성 완료")
} catch {
    print("세그먼트 생성 실패: \(error)")
}
```

### 4. 실시간 분석

```swift
// 실시간 포즈 분석
do {
    let result = try segmentManager.analyze(currentPose)
    
    print("진행도: \(result.progress * 100)%")
    print("완료 여부: \(result.isComplete)")
    print("유사도: \(result.similarity * 100)%")
    
    // 교정 피드백 생성
    for (index, correction) in result.corrections.enumerated() {
        if correction.x > 0.1 || correction.y > 0.1 || correction.z > 0.1 {
            print("관절 \(index): 교정 필요")
        }
    }
} catch {
    print("분석 실패: \(error)")
}
```

## 고급 사용법

### 1. 간편한 세그먼트 생성

```swift
// 기본 관절들로 세그먼트 생성
do {
    try segmentManager.createSegmentWithDefaultJoints(
        startKeypose: startKeypose,
        endKeypose: endKeypose
    )
} catch {
    print("세그먼트 생성 실패: \(error)")
}
```

### 2. 피드백과 함께 분석

```swift
do {
    let feedback = try segmentManager.analyzeWithFeedback(currentPose)
    
    print("진행도: \(feedback.progress * 100)%")
    print("완료: \(feedback.isComplete)")
    print("유사도: \(feedback.similarity * 100)%")
    
    // 교정 피드백 출력
    for correction in feedback.corrections {
        print("💡 \(correction)")
    }
} catch {
    print("분석 실패: \(error)")
}
```

### 3. GoogleMLKit과 연동

```swift
import MLKit

class ExerciseViewController: UIViewController {
    private let segmentManager = ExerciseSegmentManager()
    private let poseDetector = PoseDetector.poseDetector(options: PoseDetectorOptions())
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // API 초기화
        do {
            try segmentManager.initialize()
        } catch {
            print("초기화 실패: \(error)")
        }
    }
    
    func processPose(_ pose: Pose) {
        // 실시간 분석
        do {
            let result = try segmentManager.analyze(pose)
            
            // UI 업데이트
            DispatchQueue.main.async {
                self.updateProgress(result.progress)
                self.showFeedback(result)
            }
        } catch {
            print("분석 실패: \(error)")
        }
    }
    
    private func updateProgress(_ progress: Float) {
        // 진행도 바 업데이트
        progressBar.progress = progress
    }
    
    private func showFeedback(_ result: SegmentOutput) {
        // 피드백 표시
        if result.isComplete {
            showCompletionMessage()
        } else {
            showCorrections(result.corrections)
        }
    }
}
```

## 에러 처리

```swift
do {
    try segmentManager.initialize()
    try segmentManager.calibrate(with: basePose)
    try segmentManager.createSegment(startKeypose: start, endKeypose: end, careJoints: joints)
    let result = try segmentManager.analyze(currentPose)
} catch ExerciseSegmentError.initializationFailed {
    print("API 초기화에 실패했습니다.")
} catch ExerciseSegmentError.calibrationFailed {
    print("캘리브레이션에 실패했습니다.")
} catch ExerciseSegmentError.segmentCreationFailed {
    print("세그먼트 생성에 실패했습니다.")
} catch ExerciseSegmentError.analysisFailed {
    print("분석에 실패했습니다.")
} catch {
    print("알 수 없는 오류: \(error)")
}
```

## 관절 타입

```swift
enum JointType: Int32 {
    case nose = 0
    case leftEye = 1
    case rightEye = 2
    case leftEar = 3
    case rightEar = 4
    case leftShoulder = 5
    case rightShoulder = 6
    case leftElbow = 7
    case rightElbow = 8
    case leftWrist = 9
    case rightWrist = 10
    case leftHip = 11
    case rightHip = 12
}
```

## 성능 최적화

### 1. 메모리 관리

```swift
class ExerciseSession {
    private let segmentManager = ExerciseSegmentManager()
    
    deinit {
        // 자동으로 메모리 해제됨
    }
}
```

### 2. 배치 처리

```swift
// 여러 포즈를 한 번에 처리
func processBatch(_ poses: [Pose]) {
    for pose in poses {
        do {
            let result = try segmentManager.analyze(pose)
            // 결과 처리
        } catch {
            print("배치 처리 중 오류: \(error)")
        }
    }
}
```

## 문제 해결

### 1. 초기화 실패

```swift
// 상태 확인
if !segmentManager.isReady {
    print("API가 준비되지 않았습니다.")
    // 재초기화 시도
}
```

### 2. 캘리브레이션 실패

```swift
// 포즈 유효성 확인
if pose.landmark(ofType: .leftShoulder).inFrameLikelihood < 0.7 {
    print("포즈 신뢰도가 낮습니다.")
}
```

### 3. 분석 실패

```swift
// 세그먼트 생성 여부 확인
if !segmentManager.isReady {
    print("세그먼트가 생성되지 않았습니다.")
}
```

## 예제 프로젝트

완전한 예제는 `ios_app/` 디렉토리에서 확인할 수 있습니다.

## 라이선스

MIT License - 자세한 내용은 LICENSE 파일을 참조하세요.

## 지원

문제가 있거나 질문이 있으시면 GitHub Issues를 통해 문의해주세요.
