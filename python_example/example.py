#!/usr/bin/env python3
"""
Exercise Segment API Python 사용 예제
"""

from exercise_segment import (
    ExerciseSegmentAPI, 
    create_sample_pose, 
    create_squat_start_pose, 
    create_squat_end_pose, 
    create_intermediate_pose,
    JointType,
    SegmentInput
)

def run_example():
    """메인 예제 실행 함수"""
    
    print("=== Exercise Segment API Python 예제 ===\n")
    
    # Context manager를 사용하여 자동 정리
    with ExerciseSegmentAPI() as api:
        
        # 1. API 초기화
        print("1. API 초기화 중...")
        if not api.initialize():
            print("❌ 초기화 실패!")
            return
        print("✅ 초기화 성공!\n")
        
        # 2. 기본 포즈 생성
        print("2. 기본 포즈 생성 중...")
        base_pose = create_sample_pose()
        print("✅ 기본 포즈 생성 완료\n")
        
        # 3. 캘리브레이션
        print("3. 캘리브레이션 수행 중...")
        calibration = api.calibrate(base_pose)
        if calibration is None:
            print("❌ 캘리브레이션 실패!")
            return
        
        print("✅ 캘리브레이션 성공!")
        print(f"   - 스케일 팩터: {calibration.scale_factor:.2f}")
        print(f"   - 캘리브레이션 품질: {calibration.calibration_quality:.2f}")
        print(f"   - 중심점 오프셋: ({calibration.center_offset.x:.2f}, {calibration.center_offset.y:.2f}, {calibration.center_offset.z:.2f})\n")
        
        # 4. 세그먼트 생성
        print("4. 스쿼트 세그먼트 생성 중...")
        start_pose = create_squat_start_pose()
        end_pose = create_squat_end_pose()
        care_joints = [JointType.LEFT_KNEE, JointType.RIGHT_KNEE, JointType.LEFT_HIP, JointType.RIGHT_HIP]
        
        if not api.create_segment(start_pose, end_pose, calibration, care_joints):
            print("❌ 세그먼트 생성 실패!")
            return
        print("✅ 세그먼트 생성 성공!\n")
        
        # 5. 실시간 분석 시뮬레이션
        print("5. 실시간 분석 시뮬레이션...")
        print("진행도 | 완료 | 유사도 | 교정 벡터 (무릎, 골반)")
        print("-------|------|--------|----------------------")
        
        test_progresses = [0.0, 0.25, 0.5, 0.75, 1.0]
        
        for progress in test_progresses:
            current_pose = create_intermediate_pose(progress)
            input_data = SegmentInput(raw_pose=current_pose)
            output = api.analyze(input_data)
            
            if output is None:
                print("❌ 분석 실패!")
                continue
            
            print(f"{output.progress:.2f}   | {'예' if output.completed else '아니오'}   | {output.similarity:.2f}   | ", end="")
            
            # 관절별 교정 벡터 출력
            for joint in care_joints:
                correction = output.corrections[joint.value]
                print(f"({correction.x:.1f},{correction.y:.1f},{correction.z:.1f}) ", end="")
            print()
        
        print()
        
        # 6. 포즈 유효성 검사
        print("6. 포즈 유효성 검사...")
        valid_pose = create_sample_pose()
        
        # 무효한 포즈 생성 (낮은 신뢰도)
        invalid_joints = [create_sample_pose().joints[0]] * 13  # 모든 관절이 동일
        invalid_confidence = [0.3] * 13  # 낮은 신뢰도
        invalid_pose = type(valid_pose)(joints=invalid_joints, confidence=invalid_confidence, timestamp=0)
        
        valid_result = api.validate_pose(valid_pose)
        invalid_result = api.validate_pose(invalid_pose)
        
        print(f"유효한 포즈 검사: {'통과' if valid_result else '실패'}")
        print(f"무효한 포즈 검사: {'통과' if not invalid_result else '실패'}\n")
        
        # 7. 유사도 계산
        print("7. 포즈 유사도 계산...")
        pose1 = create_sample_pose()
        pose2 = create_sample_pose()  # 동일한 포즈
        pose3 = create_squat_end_pose()  # 다른 포즈
        
        similarity1 = api.calculate_similarity(pose1, pose2)
        similarity2 = api.calculate_similarity(pose1, pose3)
        
        print(f"동일 포즈 유사도: {similarity1:.2f}")
        print(f"다른 포즈 유사도: {similarity2:.2f}\n")
        
        # 8. 캘리브레이션 유효성 검사
        print("8. 캘리브레이션 유효성 검사...")
        calibration_valid = api.validate_calibration(calibration)
        print(f"캘리브레이션 유효성: {'통과' if calibration_valid else '실패'}\n")
        
        print("=== 예제 완료 ===")

def run_realtime_simulation():
    """실시간 시뮬레이션 예제"""
    
    print("\n=== 실시간 시뮬레이션 예제 ===\n")
    
    with ExerciseSegmentAPI() as api:
        if not api.initialize():
            print("❌ 초기화 실패!")
            return
        
        # 캘리브레이션
        base_pose = create_sample_pose()
        calibration = api.calibrate(base_pose)
        if calibration is None:
            print("❌ 캘리브레이션 실패!")
            return
        
        # 세그먼트 생성
        start_pose = create_squat_start_pose()
        end_pose = create_squat_end_pose()
        care_joints = [JointType.LEFT_KNEE, JointType.RIGHT_KNEE, JointType.LEFT_HIP, JointType.RIGHT_HIP]
        
        if not api.create_segment(start_pose, end_pose, calibration, care_joints):
            print("❌ 세그먼트 생성 실패!")
            return
        
        print("실시간 분석 시작... (스쿼트 운동 시뮬레이션)")
        print("Frame | 진행도 (목표) | 완료 | 유사도 | 교정 벡터")
        print("------|---------------|------|--------|----------")
        
        # 스쿼트 운동 시뮬레이션 (0.0 → 1.0 → 0.0)
        total_frames = 60  # 1초간 60fps
        import time
        
        for frame in range(total_frames):
            # 운동 진행도 계산 (0.0 → 1.0 → 0.0)
            if frame < total_frames // 2:
                # 내려가는 동작 (0.0 → 1.0)
                target_progress = frame / (total_frames // 2)
            else:
                # 올라오는 동작 (1.0 → 0.0)
                target_progress = 2.0 - frame / (total_frames // 2)
            
            # 실시간 포즈 생성
            current_pose = create_intermediate_pose(target_progress)
            input_data = SegmentInput(raw_pose=current_pose)
            output = api.analyze(input_data)
            
            if output is not None:
                print(f"Frame {frame+1:2d} | {output.progress:.2f} ({target_progress:.2f}) | {'예' if output.completed else '아니오'} | {output.similarity:.2f} | ", end="")
                
                # 주요 관절 교정 벡터 출력
                for joint in care_joints:
                    correction = output.corrections[joint.value]
                    print(f"({correction.x:.1f},{correction.y:.1f},{correction.z:.1f}) ", end="")
                print()
            
            # 실시간 시뮬레이션을 위한 지연 (실제로는 16ms)
            time.sleep(0.016)  # 16ms = 60fps
        
        print("\n✅ 실시간 시뮬레이션 완료!")

if __name__ == "__main__":
    try:
        # 기본 예제 실행
        run_example()
        
        # 실시간 시뮬레이션 실행
        run_realtime_simulation()
        
    except Exception as e:
        print(f"❌ 에러 발생: {e}")
        import traceback
        traceback.print_exc()
