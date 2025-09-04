//
//  ExerciseSegmentExample.swift
//  Swift 사용 예제
//
//  Created by Exercise Segment API Team
//  Copyright © 2024. All rights reserved.
//

import Foundation

class ExerciseSegmentExample {
    
    private let api = ExerciseSegmentAPI()
    
    func runExample() {
        do {
            // 1. API 초기화
            print("=== Exercise Segment API Swift 예제 ===")
            print("1. API 초기화 중...")
            try api.initialize()
            print("✅ 초기화 성공!")
            
            // 2. 기본 포즈 생성
            print("\n2. 기본 포즈 생성 중...")
            let basePose = createSamplePose()
            print("✅ 기본 포즈 생성 완료")
            
            // 3. 캘리브레이션
            print("\n3. 캘리브레이션 수행 중...")
            let calibration = try api.calibrate(basePose: basePose)
            print("✅ 캘리브레이션 성공!")
            print("   - 스케일 팩터: \(String(format: "%.2f", calibration.scaleFactor))")
            print("   - 캘리브레이션 품질: \(String(format: "%.2f", calibration.calibrationQuality))")
            print("   - 중심점 오프셋: (\(String(format: "%.2f", calibration.centerOffset.x)), \(String(format: "%.2f", calibration.centerOffset.y)), \(String(format: "%.2f", calibration.centerOffset.z)))")
            
            // 4. 세그먼트 생성
            print("\n4. 스쿼트 세그먼트 생성 중...")
            let startPose = createSquatStartPose()
            let endPose = createSquatEndPose()
            let careJoints: [JointType] = [.leftKnee, .rightKnee, .leftHip, .rightHip]
            
            try api.createSegment(
                startKeypose: startPose,
                endKeypose: endPose,
                calibration: calibration,
                careJoints: careJoints
            )
            print("✅ 세그먼트 생성 성공!")
            
            // 5. 실시간 분석 시뮬레이션
            print("\n5. 실시간 분석 시뮬레이션...")
            print("진행도 | 완료 | 유사도 | 교정 벡터 (무릎, 골반)")
            print("-------|------|--------|----------------------")
            
            let testProgresses: [Float] = [0.0, 0.25, 0.5, 0.75, 1.0]
            
            for progress in testProgresses {
                let currentPose = createIntermediatePose(progress: progress)
                let input = SegmentInput(rawPose: currentPose)
                let output = try api.analyze(input: input)
                
                print("\(String(format: "%.2f", output.progress))   | \(output.completed ? "예" : "아니오")   | \(String(format: "%.2f", output.similarity))   | ", terminator: "")
                
                // 관절별 교정 벡터 출력
                for joint in careJoints {
                    let correction = output.corrections[joint.rawValue]
                    print("(\(String(format: "%.1f", correction.x)),\(String(format: "%.1f", correction.y)),\(String(format: "%.1f", correction.z))) ", terminator: "")
                }
                print()
            }
            
            // 6. 포즈 유효성 검사
            print("\n6. 포즈 유효성 검사...")
            let validPose = createSamplePose()
            let invalidPose = createInvalidPose()
            
            let validResult = api.validatePose(validPose)
            let invalidResult = api.validatePose(invalidPose)
            
            print("유효한 포즈 검사: \(validResult ? "통과" : "실패")")
            print("무효한 포즈 검사: \(!invalidResult ? "통과" : "실패")")
            
            // 7. 유사도 계산
            print("\n7. 포즈 유사도 계산...")
            let pose1 = createSamplePose()
            let pose2 = createSamplePose() // 동일한 포즈
            let pose3 = createSquatEndPose() // 다른 포즈
            
            let similarity1 = api.calculateSimilarity(pose1: pose1, pose2: pose2)
            let similarity2 = api.calculateSimilarity(pose1: pose1, pose2: pose3)
            
            print("동일 포즈 유사도: \(String(format: "%.2f", similarity1))")
            print("다른 포즈 유사도: \(String(format: "%.2f", similarity2))")
            
            print("\n=== 예제 완료 ===")
            
        } catch let error as ExerciseSegmentError {
            print("❌ 에러 발생: \(error)")
        } catch {
            print("❌ 알 수 없는 에러: \(error)")
        }
    }
    
    // MARK: - Helper Methods
    
    private func createSamplePose() -> PoseData {
        let joints = [
            Point3D(x: 0.0, y: -10.0, z: 0.0),      // nose
            Point3D(x: -20.0, y: 0.0, z: 0.0),      // left_shoulder
            Point3D(x: 20.0, y: 0.0, z: 0.0),       // right_shoulder
            Point3D(x: -30.0, y: 20.0, z: 0.0),     // left_elbow
            Point3D(x: 30.0, y: 20.0, z: 0.0),      // right_elbow
            Point3D(x: -40.0, y: 40.0, z: 0.0),     // left_wrist
            Point3D(x: 40.0, y: 40.0, z: 0.0),      // right_wrist
            Point3D(x: -10.0, y: 50.0, z: 0.0),     // left_hip
            Point3D(x: 10.0, y: 50.0, z: 0.0),      // right_hip
            Point3D(x: -10.0, y: 80.0, z: 0.0),     // left_knee
            Point3D(x: 10.0, y: 80.0, z: 0.0),      // right_knee
            Point3D(x: -10.0, y: 110.0, z: 0.0),    // left_ankle
            Point3D(x: 10.0, y: 110.0, z: 0.0)      // right_ankle
        ]
        
        let confidence = Array(repeating: 0.9, count: 13)
        let timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        
        return PoseData(joints: joints, confidence: confidence, timestamp: timestamp)
    }
    
    private func createSquatStartPose() -> PoseData {
        return createSamplePose() // 시작 포즈는 기본 포즈와 동일
    }
    
    private func createSquatEndPose() -> PoseData {
        var joints = createSamplePose().joints
        
        // 무릎을 구부리고 골반을 낮춤
        joints[9] = Point3D(x: joints[9].x, y: joints[9].y + 30.0, z: joints[9].z)  // left_knee
        joints[10] = Point3D(x: joints[10].x, y: joints[10].y + 30.0, z: joints[10].z) // right_knee
        joints[7] = Point3D(x: joints[7].x, y: joints[7].y + 20.0, z: joints[7].z)  // left_hip
        joints[8] = Point3D(x: joints[8].x, y: joints[8].y + 20.0, z: joints[8].z)  // right_hip
        
        let confidence = Array(repeating: 0.9, count: 13)
        let timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        
        return PoseData(joints: joints, confidence: confidence, timestamp: timestamp)
    }
    
    private func createIntermediatePose(progress: Float) -> PoseData {
        let startJoints = createSquatStartPose().joints
        let endJoints = createSquatEndPose().joints
        
        var intermediateJoints: [Point3D] = []
        
        for i in 0..<13 {
            let startJoint = startJoints[i]
            let endJoint = endJoints[i]
            
            let x = startJoint.x + progress * (endJoint.x - startJoint.x)
            let y = startJoint.y + progress * (endJoint.y - startJoint.y)
            let z = startJoint.z + progress * (endJoint.z - startJoint.z)
            
            intermediateJoints.append(Point3D(x: x, y: y, z: z))
        }
        
        let confidence = Array(repeating: 0.9, count: 13)
        let timestamp = UInt64(Date().timeIntervalSince1970 * 1000)
        
        return PoseData(joints: intermediateJoints, confidence: confidence, timestamp: timestamp)
    }
    
    private func createInvalidPose() -> PoseData {
        let joints = Array(repeating: Point3D(x: 0, y: 0, z: 0), count: 13)
        let confidence = Array(repeating: 0.3, count: 13) // 낮은 신뢰도
        let timestamp: UInt64 = 0 // 잘못된 타임스탬프
        
        return PoseData(joints: joints, confidence: confidence, timestamp: timestamp)
    }
}

// MARK: - 실행 예제

// 사용법:
// let example = ExerciseSegmentExample()
// example.runExample()
