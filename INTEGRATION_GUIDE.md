# Exercise Segment API - 다른 언어 통합 가이드

현재 완성된 C API를 다양한 언어와 플랫폼에서 사용하는 방법을 설명합니다.

## 🎯 현재 상태

- ✅ **완전한 C API** 구현 완료
- ✅ **정적 라이브러리** (`libexercise_segment.a`) 생성
- ✅ **헤더 파일들** (`include/` 디렉토리)
- ✅ **테스트 및 예제** 완성

## 📱 1. Swift (iOS/macOS) 통합

### ⚠️ Swift 사용 시 주의사항

Swift에서 C API를 사용할 때 다음과 같은 타입 변환 문제가 발생할 수 있습니다:

#### 문제 1: 타입 변환 오류
```
Cannot convert value of type 'Swift.UnsafePointer<FisicaWorkout.PoseData>' 
to expected argument type 'Swift.UnsafePointer<__ObjC.PoseData>'
```

#### 문제 2: __ObjC 타입 인식 오류
```
Cannot find type '__ObjC' in scope
```

#### 해결 방법

이러한 문제들은 **Bridging Header**에서 타입 별칭을 정의하여 해결되었습니다:

```objc
// FisicaWorkout-Bridging-Header.h
#import "segment_api.h"
#import "segment_types.h"

// Swift에서 타입 충돌을 방지하기 위한 별칭
typedef PoseData CAPoseData;
typedef PoseLandmark CAPoseLandmark;
typedef Point3D CAPoint3D;
typedef SegmentOutput CASegmentOutput;
typedef PoseLandmarkType CAPoseLandmarkType;
```

**핵심 해결책:**
1. **타입 별칭 사용**: `PoseData` → `CAPoseData`로 별칭을 만들어 Swift에서 명확하게 구분
2. **Bridging Header 활용**: Objective-C 브릿지를 통해 C 타입을 Swift에서 안전하게 사용
3. **withUnsafePointer 사용**: Swift에서 C 함수에 포인터를 안전하게 전달

### 방법 1: Objective-C Bridge 사용

#### 1.1 Xcode 프로젝트 설정
```bash
# 1. 라이브러리와 헤더를 Xcode 프로젝트에 추가
# - libexercise_segment.a
# - include/ 디렉토리의 모든 .h 파일들
```

#### 1.2 Bridging Header 생성
```objc
// ExerciseSegment-Bridging-Header.h
#import "segment_api.h"
#import "segment_types.h"
```

#### 1.3 Swift에서 안전한 사용법
```swift
import Foundation
import GoogleMLKit

class ExerciseSegmentManager {
    private var isInitialized = false
    
    func initialize() -> Bool {
        let result = segment_api_init()
        isInitialized = (result == SEGMENT_OK)
        return isInitialized
    }
    
    func calibrateRecorder(with pose: Pose) -> Bool {
        guard isInitialized else { return false }
        
        let poseData = convertMLKitPoseToPoseData(pose)
        
        let result = withUnsafePointer(to: poseData) { cPoseData in
            segment_calibrate_recorder(cPoseData)
        }
        
        return result == Int32(SEGMENT_OK.rawValue)
    }
    
    func recordPose(_ pose: Pose, name: String, jsonFile: String) -> Bool {
        guard isInitialized else { return false }
        
        let poseData = convertMLKitPoseToPoseData(pose)
        
        let result = withUnsafePointer(to: poseData) { cPoseData in
            segment_record_pose(cPoseData, name, jsonFile)
        }
        
        return result == Int32(SEGMENT_OK.rawValue)
    }
    
    deinit {
        if isInitialized {
            segment_api_cleanup()
        }
    }
}
```

#### 1.4 Google ML Kit와의 통합 (실제 구현)
```swift
import GoogleMLKit

extension ExerciseSegmentManager {
    /// Google ML Kit Pose를 CAPoseData로 변환 (실제 해결된 방법)
    private func convertMLKitPoseToPoseData(_ pose: Pose) -> CAPoseData {
        var poseData = CAPoseData()
        
        // MLKit의 33개 랜드마크를 C API의 33개 랜드마크로 매핑
        let landmarkMapping: [(MLKit.PoseLandmarkType, Int32)] = [
            // 얼굴 (11개)
            (.nose, 0), (.leftEyeInner, 1), (.leftEye, 2), (.leftEyeOuter, 3),
            (.rightEyeInner, 4), (.rightEye, 5), (.rightEyeOuter, 6),
            (.leftEar, 7), (.rightEar, 8), (.mouthLeft, 9), (.mouthRight, 10),
            
            // 상체 (12개)
            (.leftShoulder, 11), (.rightShoulder, 12), (.leftElbow, 13), (.rightElbow, 14),
            (.leftWrist, 15), (.rightWrist, 16), (.leftPinkyFinger, 17), (.rightPinkyFinger, 18),
            (.leftIndexFinger, 19), (.rightIndexFinger, 20), (.leftThumb, 21), (.rightThumb, 22),
            
            // 하체 (10개)
            (.leftHip, 23), (.rightHip, 24), (.leftKnee, 25), (.rightKnee, 26),
            (.leftAnkle, 27), (.rightAnkle, 28), (.leftHeel, 29), (.rightHeel, 30),
            (.leftAnkle, 31), (.rightAnkle, 32)  // 발가락 없음, 발목 재사용
        ]
        
        for (mlKitType, cType) in landmarkMapping {
            let landmark = pose.landmark(ofType: mlKitType)
            
            // C API 구조체에 직접 할당 (Bridging Header의 별칭 사용)
            poseData.landmarks[Int(cType)].position.x = Float(landmark.position.x)
            poseData.landmarks[Int(cType)].position.y = Float(landmark.position.y)
            poseData.landmarks[Int(cType)].position.z = 0.0  // ML Kit는 2D만 제공
            poseData.landmarks[Int(cType)].inFrameLikelihood = landmark.inFrameLikelihood
        }
        
        poseData.timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        return poseData
    }
}
```

**핵심 포인트:**
1. **CAPoseData 사용**: Bridging Header에서 정의한 별칭 타입 사용
2. **직접 구조체 접근**: `poseData.landmarks[Int(cType)]`로 직접 배열에 접근
3. **withUnsafePointer**: C 함수 호출 시 안전한 포인터 전달

### 방법 2: Swift Package Manager 사용

#### 2.1 Package.swift 생성
```swift
// Package.swift
import PackageDescription

let package = Package(
    name: "ExerciseSegmentAPI",
    platforms: [
        .iOS(.v12),
        .macOS(.v10_15)
    ],
    products: [
        .library(name: "ExerciseSegmentAPI", targets: ["ExerciseSegmentAPI"])
    ],
    targets: [
        .target(
            name: "ExerciseSegmentAPI",
            dependencies: [],
            path: "Sources",
            publicHeadersPath: "include"
        )
    ]
)
```

## 🐍 2. Python 통합

### 방법 1: ctypes 사용

#### 2.1 Python 래퍼 생성
```python
# exercise_segment.py
import ctypes
import ctypes.util
from ctypes import Structure, c_float, c_int, c_bool, c_uint64, POINTER

# 라이브러리 로드
lib = ctypes.CDLL('./libexercise_segment.so')  # Linux
# lib = ctypes.CDLL('./libexercise_segment.dylib')  # macOS
# lib = ctypes.CDLL('./exercise_segment.dll')  # Windows

# 구조체 정의
class Point3D(Structure):
    _fields_ = [("x", c_float), ("y", c_float), ("z", c_float)]

class PoseData(Structure):
    _fields_ = [
        ("joints", Point3D * 13),
        ("confidence", c_float * 13),
        ("timestamp", c_uint64)
    ]

class CalibrationData(Structure):
    _fields_ = [
        ("scale_factor", c_float),
        ("center_offset", Point3D),
        ("is_calibrated", c_bool),
        ("calibration_quality", c_float)
    ]

class SegmentInput(Structure):
    _fields_ = [("raw_pose", PoseData)]

class SegmentOutput(Structure):
    _fields_ = [
        ("progress", c_float),
        ("completed", c_bool),
        ("similarity", c_float),
        ("corrections", Point3D * 13),
        ("timestamp", c_uint64)
    ]

# 함수 시그니처 설정
lib.segment_api_init.restype = c_int
lib.segment_calibrate.argtypes = [POINTER(PoseData), POINTER(CalibrationData)]
lib.segment_calibrate.restype = c_int
lib.segment_analyze.argtypes = [POINTER(SegmentInput)]
lib.segment_analyze.restype = SegmentOutput

class ExerciseSegmentAPI:
    def __init__(self):
        self.initialized = False
    
    def init(self):
        result = lib.segment_api_init()
        self.initialized = (result == 0)
        return self.initialized
    
    def calibrate(self, base_pose):
        if not self.initialized:
            return None
        
        calibration = CalibrationData()
        result = lib.segment_calibrate(base_pose, calibration)
        return calibration if result == 0 else None
    
    def analyze(self, input_data):
        if not self.initialized:
            return None
        return lib.segment_analyze(input_data)
    
    def cleanup(self):
        if self.initialized:
            lib.segment_api_cleanup()
            self.initialized = False

# 사용 예제
if __name__ == "__main__":
    api = ExerciseSegmentAPI()
    if api.init():
        print("API 초기화 성공!")
        # 사용...
        api.cleanup()
```

### 방법 2: Cython 사용

#### 2.2 Cython 래퍼
```cython
# exercise_segment_cython.pyx
cimport cython
from libc.stdint cimport uint64_t

cdef extern from "segment_api.h":
    ctypedef struct Point3D:
        float x, y, z
    
    ctypedef struct PoseData:
        Point3D joints[13]
        float confidence[13]
        uint64_t timestamp
    
    ctypedef struct CalibrationData:
        float scale_factor
        Point3D center_offset
        bint is_calibrated
        float calibration_quality
    
    int segment_api_init()
    int segment_calibrate(PoseData* base_pose, CalibrationData* out_calibration)
    SegmentOutput segment_analyze(SegmentInput* input)
    void segment_api_cleanup()

class ExerciseSegmentAPI:
    cdef bint initialized
    
    def __init__(self):
        self.initialized = False
    
    def init(self):
        cdef int result = segment_api_init()
        self.initialized = (result == 0)
        return self.initialized
    
    def calibrate(self, base_pose):
        if not self.initialized:
            return None
        
        cdef CalibrationData calibration
        cdef int result = segment_calibrate(&base_pose, &calibration)
        return calibration if result == 0 else None
```

## 🎯 3. Flutter (Dart) 통합

### 방법 1: Platform Channels 사용

#### 3.1 Native iOS/Android 구현
```dart
// exercise_segment.dart
import 'dart:ffi';
import 'dart:io';

class ExerciseSegmentAPI {
  static final ExerciseSegmentAPI _instance = ExerciseSegmentAPI._internal();
  factory ExerciseSegmentAPI() => _instance;
  ExerciseSegmentAPI._internal();

  late DynamicLibrary _lib;
  bool _initialized = false;

  Future<bool> initialize() async {
    if (Platform.isIOS) {
      _lib = DynamicLibrary.process();
    } else if (Platform.isAndroid) {
      _lib = DynamicLibrary.open('libexercise_segment.so');
    }
    
    final initFunc = _lib.lookupFunction<Int32 Function(), int Function()>('segment_api_init');
    final result = initFunc();
    _initialized = (result == 0);
    return _initialized;
  }

  Future<Map<String, dynamic>?> calibrate(Map<String, dynamic> basePose) async {
    if (!_initialized) return null;
    
    // Native method channel을 통해 구현
    final result = await _channel.invokeMethod('calibrate', basePose);
    return result;
  }

  Future<Map<String, dynamic>?> analyze(Map<String, dynamic> input) async {
    if (!_initialized) return null;
    
    final result = await _channel.invokeMethod('analyze', input);
    return result;
  }
}
```

#### 3.2 iOS Native 구현 (Swift)
```swift
// ExerciseSegmentPlugin.swift
import Flutter
import Foundation

public class ExerciseSegmentPlugin: NSObject, FlutterPlugin {
    private var api: ExerciseSegmentManager?
    
    public static func register(with registrar: FlutterPluginRegistrar) {
        let channel = FlutterMethodChannel(name: "exercise_segment", binaryMessenger: registrar.messenger())
        let instance = ExerciseSegmentPlugin()
        registrar.addMethodCallDelegate(instance, channel: channel)
    }
    
    public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
        switch call.method {
        case "initialize":
            api = ExerciseSegmentManager()
            let success = api?.initialize() ?? false
            result(success)
            
        case "calibrate":
            guard let args = call.arguments as? [String: Any],
                  let api = api else {
                result(FlutterError(code: "INVALID_ARGS", message: "Invalid arguments", details: nil))
                return
            }
            
            let basePose = convertToPoseData(args)
            let calibration = api.calibrate(basePose: basePose)
            result(convertFromCalibrationData(calibration))
            
        case "analyze":
            guard let args = call.arguments as? [String: Any],
                  let api = api else {
                result(FlutterError(code: "INVALID_ARGS", message: "Invalid arguments", details: nil))
                return
            }
            
            let input = convertToSegmentInput(args)
            let output = api.analyze(input: input)
            result(convertFromSegmentOutput(output))
            
        default:
            result(FlutterMethodNotImplemented)
        }
    }
}
```

## 🌐 4. Web (JavaScript) 통합

### 방법 1: WebAssembly 사용

#### 4.1 Emscripten으로 컴파일
```bash
# C API를 WebAssembly로 컴파일
emcc src/*.c -I include -s EXPORTED_FUNCTIONS="['_segment_api_init', '_segment_calibrate', '_segment_analyze']" -s EXPORTED_RUNTIME_METHODS="['ccall', 'cwrap']" -o exercise_segment.js
```

#### 4.2 JavaScript 래퍼
```javascript
// exercise-segment.js
class ExerciseSegmentAPI {
    constructor() {
        this.module = null;
        this.initialized = false;
    }

    async load() {
        this.module = await Module();
        return this;
    }

    init() {
        if (!this.module) return false;
        
        const result = this.module.ccall('segment_api_init', 'number', [], []);
        this.initialized = (result === 0);
        return this.initialized;
    }

    calibrate(basePose) {
        if (!this.initialized) return null;
        
        // JavaScript 객체를 C 구조체로 변환
        const posePtr = this.convertToPoseData(basePose);
        const calibrationPtr = this.module._malloc(32); // CalibrationData 크기
        
        const result = this.module.ccall('segment_calibrate', 'number', 
            ['number', 'number'], [posePtr, calibrationPtr]);
        
        if (result === 0) {
            const calibration = this.convertFromCalibrationData(calibrationPtr);
            this.module._free(posePtr);
            this.module._free(calibrationPtr);
            return calibration;
        }
        
        this.module._free(posePtr);
        this.module._free(calibrationPtr);
        return null;
    }

    analyze(input) {
        if (!this.initialized) return null;
        
        const inputPtr = this.convertToSegmentInput(input);
        const output = this.module.ccall('segment_analyze', 'number', ['number'], [inputPtr]);
        
        this.module._free(inputPtr);
        return this.convertFromSegmentOutput(output);
    }
}

// 사용 예제
const api = new ExerciseSegmentAPI();
await api.load();
if (api.init()) {
    console.log("API 초기화 성공!");
    // 사용...
}
```

## 🔧 5. 빌드 및 배포 가이드

### 5.1 크로스 플랫폼 빌드
```bash
# iOS (ARM64)
cmake -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 -DCMAKE_OSX_ARCHITECTURES=arm64 ..

# Android (ARM64)
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21 ..

# Windows (x64)
cmake -G "Visual Studio 16 2019" -A x64 ..

# Linux (x64)
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### 5.2 패키지 생성
```bash
# 정적 라이브러리
make

# 동적 라이브러리 (공유 라이브러리)
cmake -DBUILD_SHARED_LIBS=ON ..
make

# 설치
make install
```

## 📋 6. 통합 체크리스트

### 공통 요구사항
- [ ] C API 헤더 파일들 (`include/` 디렉토리)
- [ ] 컴파일된 라이브러리 (`.a`, `.so`, `.dll`, `.dylib`)
- [ ] 플랫폼별 빌드 도구
- [ ] 메모리 관리 (초기화/정리)

### 플랫폼별 추가 요구사항

#### iOS/macOS
- [ ] Xcode 프로젝트 설정
- [ ] Bridging Header
- [ ] Framework 타겟 (선택사항)

#### Android
- [ ] NDK 설정
- [ ] JNI 래퍼 (Java/Kotlin)
- [ ] ABI 필터링

#### Python
- [ ] ctypes 또는 Cython
- [ ] pip 패키지 설정
- [ ] 플랫폼별 바이너리 배포

#### Flutter
- [ ] Platform Channels
- [ ] Native 플러그인 구현
- [ ] pub.dev 패키지 등록

#### Web
- [ ] Emscripten 빌드
- [ ] WebAssembly 모듈
- [ ] npm 패키지 배포

## 🚀 7. 다음 단계

1. **라이브러리 최적화**: 크기 최적화, 성능 튜닝
2. **패키지 관리**: 각 플랫폼별 패키지 매니저 지원
3. **문서화**: API 문서, 예제 코드
4. **CI/CD**: 자동 빌드 및 테스트
5. **배포**: 각 플랫폼 스토어/레지스트리 등록

이제 완성된 C API를 다양한 언어와 플랫폼에서 활용할 수 있습니다! 🎉
