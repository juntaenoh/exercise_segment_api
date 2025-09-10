# ExerciseSegmentAPI Swift 사용법 v2.0.0

## 개요

ExerciseSegmentAPI v2.0.0은 Swift에서 GoogleMLKit과 함께 사용할 수 있도록 최적화된 운동 세그먼트 분석 라이브러리입니다. 새로운 버전에서는 A(기록자)와 B(사용자) 역할로 API가 분리되어 더욱 유연한 워크아웃 관리가 가능합니다.

## 설치

### CocoaPods

```ruby
pod 'ExerciseSegmentAPI', '~> 2.0'
```

### 의존성

- GoogleMLKit/PoseDetection ~> 4.0
- iOS 12.0+
- Swift 5.0+

## A 이용자 (기록자) - 워크아웃 생성

### 1. 초기화 및 캘리브레이션

```swift
import ExerciseSegmentAPI
import MLKit

class WorkoutRecorder: ObservableObject {
    private let segmentManager = ExerciseSegmentManager()
    
    func setupAsRecorder() throws {
        // API 초기화
        try segmentManager.initialize()
        
        // A의 기본 포즈로 캘리브레이션
        let myBasePose = getMyNaturalPose() // MLKit Pose 객체
        try segmentManager.calibrateRecorder(with: myBasePose)
        print("A 이용자 캘리브레이션 완료")
    }
}
```

### 2. 포즈 기록

```swift
func recordWorkout() throws {
    // 스쿼트 워크아웃 기록
    let standingPose = getStandingPose()
    let squatDownPose = getSquatDownPose()
    let squatUpPose = getSquatUpPose()
    
    try segmentManager.recordPose(standingPose, name: "standing", jsonFile: "squat_workout.json")
    try segmentManager.recordPose(squatDownPose, name: "squat_down", jsonFile: "squat_workout.json")
    try segmentManager.recordPose(squatUpPose, name: "squat_up", jsonFile: "squat_workout.json")
    
    // 워크아웃 완성
    try segmentManager.finalizeWorkout(name: "squat", jsonFile: "squat_workout.json")
    print("워크아웃 생성 완료")
}
```

## B 이용자 (사용자) - 워크아웃 사용

### 1. 초기화 및 캘리브레이션

```swift
class ExerciseSegmentManager: ObservableObject {
    private let segmentManager = ExerciseSegmentManager()
    
    func setupAsUser() throws {
        // API 초기화
        try segmentManager.initialize()
        
        // B의 기본 포즈로 캘리브레이션
        let myBasePose = getMyNaturalPose() // MLKit Pose 객체
        try segmentManager.calibrateUser(with: myBasePose)
        print("B 이용자 캘리브레이션 완료")
    }
}
```

### 2. 세그먼트 로드

```swift
func loadWorkoutSegment() throws {
    // 세그먼트 1: 서기 → 스쿼트 내려가기 (인덱스 0→1)
    try segmentManager.loadSegment(jsonFile: "squat_workout.json", 
                                 startIndex: 0, 
                                 endIndex: 1)
    print("세그먼트 로드 완료")
}
```

### 3. 실시간 분석

```swift
func analyzeCurrentPose(_ pose: Pose) throws -> SegmentOutput {
    let output = try segmentManager.analyze(pose)
    
    print("진행도: \(output.progress)")
    print("완료 여부: \(output.completed ? "예" : "아니오")")
    print("유사도: \(output.similarity)")
    
    return output
}

func getTargetPose() throws -> PoseData {
    return try segmentManager.getTransformedEndPose()
}
```

## 통합 사용 예제

### 완전한 워크플로우

```swift
class ExerciseApp: ObservableObject {
    private let recorder = WorkoutRecorder()
    private let user = ExerciseSegmentManager()
    
    func createAndUseWorkout() throws {
        // 1. A가 워크아웃 생성
        try recorder.setupAsRecorder()
        try recorder.recordWorkout()
        
        // 2. B가 워크아웃 사용
        try user.setupAsUser()
        try user.loadWorkoutSegment()
        
        // 3. 실시간 분석
        let currentPose = getCurrentPose()
        let result = try user.analyzeCurrentPose(currentPose)
        
        // 4. 목표 포즈 확인
        let targetPose = try user.getTargetPose()
    }
}
```

## 고급 사용법

### 1. 여러 세그먼트 순차 실행

```swift
class MultiSegmentWorkout: ObservableObject {
    private let user = ExerciseSegmentManager()
    private var currentSegmentIndex = 0
    private let segments = [(0, 1), (1, 2), (2, 0)] // 서기→내려가기→올라가기→서기
    
    func startWorkout() throws {
        try user.setupAsUser()
        try loadNextSegment()
    }
    
    func loadNextSegment() throws {
        if currentSegmentIndex < segments.count {
            let (start, end) = segments[currentSegmentIndex]
            try user.loadSegment(jsonFile: "squat_workout.json", 
                               startIndex: start, 
                               endIndex: end)
            print("세그먼트 \(currentSegmentIndex + 1) 로드 완료")
        }
    }
    
    func completeCurrentSegment() throws {
        currentSegmentIndex += 1
        if currentSegmentIndex < segments.count {
            try loadNextSegment()
        } else {
            print("워크아웃 완료!")
        }
    }
}
```

### 2. 실시간 피드백 시스템

```swift
class RealTimeFeedback: ObservableObject {
    private let user = ExerciseSegmentManager()
    @Published var currentProgress: Float = 0.0
    @Published var isCompleted: Bool = false
    @Published var feedbackMessages: [String] = []
    
    func analyzeWithFeedback(_ pose: Pose) throws {
        let result = try user.analyze(pose)
        
        currentProgress = result.progress
        isCompleted = result.completed
        
        // 교정 피드백 생성
        feedbackMessages = generateFeedbackMessages(result.corrections)
    }
    
    private func generateFeedbackMessages(_ corrections: [Point3D]) -> [String] {
        var messages: [String] = []
        
        let jointNames = ["코", "왼어깨", "오른어깨", "왼팔꿈치", "오른팔꿈치", 
                         "왼손목", "오른손목", "왼골반", "오른골반", 
                         "왼무릎", "오른무릎", "왼발목", "오른발목"]
        
        for (index, correction) in corrections.enumerated() {
            if abs(correction.x) > 0.1 || abs(correction.y) > 0.1 || abs(correction.z) > 0.1 {
                let direction = getDirectionString(correction)
                messages.append("\(jointNames[index])을 \(direction)로 움직이세요")
            }
        }
        
        return messages
    }
}
```

### 3. GoogleMLKit과 연동

```swift
import MLKit

class ExerciseViewController: UIViewController {
    private let user = ExerciseSegmentManager()
    private let poseDetector = PoseDetector.poseDetector(options: PoseDetectorOptions())
    @Published var currentProgress: Float = 0.0
    @Published var isCompleted: Bool = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        // B 이용자로 설정
        do {
            try user.setupAsUser()
            try user.loadSegment(jsonFile: "squat_workout.json", startIndex: 0, endIndex: 1)
        } catch {
            print("초기화 실패: \(error)")
        }
    }
    
    func processPose(_ pose: Pose) {
        // 실시간 분석
        do {
            let result = try user.analyze(pose)
            
            // UI 업데이트
            DispatchQueue.main.async {
                self.currentProgress = result.progress
                self.isCompleted = result.completed
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
        if result.completed {
            showCompletionMessage()
        } else {
            showCorrections(result.corrections)
        }
    }
    
    func getTargetPose() -> PoseData? {
        do {
            return try user.getTargetPose()
        } catch {
            print("목표 포즈 가져오기 실패: \(error)")
            return nil
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
