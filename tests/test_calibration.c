/**
 * @file test_calibration.c
 * @brief 캘리브레이션 함수들 테스트
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "calibration.h"
#include "segment_types.h"

#define EPSILON 0.001f

void create_test_pose(PoseData* pose, float shoulder_width, float height) {
    if (!pose) return;
    
    // 기본 관절 위치 설정
    pose->joints[JOINT_NOSE] = (Point3D){0.0f, -height/2, 0.0f};
    pose->joints[JOINT_LEFT_SHOULDER] = (Point3D){-shoulder_width/2, 0.0f, 0.0f};
    pose->joints[JOINT_RIGHT_SHOULDER] = (Point3D){shoulder_width/2, 0.0f, 0.0f};
    pose->joints[JOINT_LEFT_ELBOW] = (Point3D){-shoulder_width/2 - 10.0f, 20.0f, 0.0f};
    pose->joints[JOINT_RIGHT_ELBOW] = (Point3D){shoulder_width/2 + 10.0f, 20.0f, 0.0f};
    pose->joints[JOINT_LEFT_WRIST] = (Point3D){-shoulder_width/2 - 20.0f, 40.0f, 0.0f};
    pose->joints[JOINT_RIGHT_WRIST] = (Point3D){shoulder_width/2 + 20.0f, 40.0f, 0.0f};
    pose->joints[JOINT_LEFT_HIP] = (Point3D){-shoulder_width/4, height/2, 0.0f};
    pose->joints[JOINT_RIGHT_HIP] = (Point3D){shoulder_width/4, height/2, 0.0f};
    pose->joints[JOINT_LEFT_KNEE] = (Point3D){-shoulder_width/4, height/2 + 30.0f, 0.0f};
    pose->joints[JOINT_RIGHT_KNEE] = (Point3D){shoulder_width/4, height/2 + 30.0f, 0.0f};
    pose->joints[JOINT_LEFT_ANKLE] = (Point3D){-shoulder_width/4, height/2 + 60.0f, 0.0f};
    pose->joints[JOINT_RIGHT_ANKLE] = (Point3D){shoulder_width/4, height/2 + 60.0f, 0.0f};
    
    // 모든 관절의 신뢰도를 높게 설정
    for (int i = 0; i < JOINT_COUNT; i++) {
        pose->confidence[i] = 0.9f;
    }
    
    pose->timestamp = 1000;
}

void test_segment_calibrate() {
    printf("테스트: segment_calibrate\n");
    
    PoseData base_pose;
    create_test_pose(&base_pose, 40.0f, 170.0f);  // 표준 크기
    
    CalibrationData calibration;
    int result = segment_calibrate(&base_pose, &calibration);
    
    assert(result == SEGMENT_OK);
    assert(calibration.is_calibrated == true);
    assert(fabsf(calibration.scale_factor - 1.0f) < 0.1f);  // 표준 크기이므로 1.0에 가까움
    assert(calibration.calibration_quality > 0.6f);
    
    printf("  ✓ 표준 크기 캘리브레이션 테스트 통과\n");
    
    // 큰 체형 테스트
    create_test_pose(&base_pose, 60.0f, 200.0f);  // 큰 체형
    result = segment_calibrate(&base_pose, &calibration);
    
    assert(result == SEGMENT_OK);
    assert(calibration.is_calibrated == true);
    assert(calibration.scale_factor > 1.0f);  // 큰 체형이므로 1.0보다 큼
    assert(calibration.calibration_quality > 0.6f);
    
    printf("  ✓ 큰 체형 캘리브레이션 테스트 통과\n");
    
    // 작은 체형 테스트
    create_test_pose(&base_pose, 30.0f, 150.0f);  // 작은 체형
    result = segment_calibrate(&base_pose, &calibration);
    
    assert(result == SEGMENT_OK);
    assert(calibration.is_calibrated == true);
    assert(calibration.scale_factor < 1.0f);  // 작은 체형이므로 1.0보다 작음
    assert(calibration.calibration_quality > 0.6f);
    
    printf("  ✓ 작은 체형 캘리브레이션 테스트 통과\n");
}

void test_segment_validate_calibration() {
    printf("테스트: segment_validate_calibration\n");
    
    // 유효한 캘리브레이션
    CalibrationData valid_calibration = {
        .scale_factor = 1.0f,
        .center_offset = {0.0f, 0.0f, 0.0f},
        .is_calibrated = true,
        .calibration_quality = 0.8f
    };
    
    assert(segment_validate_calibration(&valid_calibration) == true);
    printf("  ✓ 유효한 캘리브레이션 검증 테스트 통과\n");
    
    // 무효한 캘리브레이션들
    CalibrationData invalid_calibration1 = {
        .scale_factor = 1.0f,
        .center_offset = {0.0f, 0.0f, 0.0f},
        .is_calibrated = false,  // 캘리브레이션 안됨
        .calibration_quality = 0.8f
    };
    
    assert(segment_validate_calibration(&invalid_calibration1) == false);
    printf("  ✓ 캘리브레이션 안됨 검증 테스트 통과\n");
    
    CalibrationData invalid_calibration2 = {
        .scale_factor = 0.1f,  // 너무 작은 스케일
        .center_offset = {0.0f, 0.0f, 0.0f},
        .is_calibrated = true,
        .calibration_quality = 0.8f
    };
    
    assert(segment_validate_calibration(&invalid_calibration2) == false);
    printf("  ✓ 잘못된 스케일 팩터 검증 테스트 통과\n");
    
    CalibrationData invalid_calibration3 = {
        .scale_factor = 1.0f,
        .center_offset = {0.0f, 0.0f, 0.0f},
        .is_calibrated = true,
        .calibration_quality = 0.3f  // 너무 낮은 품질
    };
    
    assert(segment_validate_calibration(&invalid_calibration3) == false);
    printf("  ✓ 낮은 품질 검증 테스트 통과\n");
}

void test_apply_calibration_to_pose() {
    printf("테스트: apply_calibration_to_pose\n");
    
    PoseData original_pose;
    create_test_pose(&original_pose, 40.0f, 170.0f);
    
    CalibrationData calibration = {
        .scale_factor = 2.0f,
        .center_offset = {10.0f, 20.0f, 30.0f},
        .is_calibrated = true,
        .calibration_quality = 0.8f
    };
    
    PoseData calibrated_pose;
    int result = apply_calibration_to_pose(&original_pose, &calibration, &calibrated_pose);
    
    assert(result == SEGMENT_OK);
    
    // 첫 번째 관절이 올바르게 변환되었는지 확인
    Point3D original_joint = original_pose.joints[0];
    Point3D calibrated_joint = calibrated_pose.joints[0];
    
    float expected_x = original_joint.x * 2.0f + 10.0f;
    float expected_y = original_joint.y * 2.0f + 20.0f;
    float expected_z = original_joint.z * 2.0f + 30.0f;
    
    assert(fabsf(calibrated_joint.x - expected_x) < EPSILON);
    assert(fabsf(calibrated_joint.y - expected_y) < EPSILON);
    assert(fabsf(calibrated_joint.z - expected_z) < EPSILON);
    
    printf("  ✓ 캘리브레이션 적용 테스트 통과\n");
}

void test_calculate_pose_scale_factor() {
    printf("테스트: calculate_pose_scale_factor\n");
    
    PoseData pose;
    create_test_pose(&pose, 60.0f, 200.0f);  // 큰 체형
    
    float scale_factor = calculate_pose_scale_factor(&pose);
    assert(scale_factor > 1.0f);  // 큰 체형이므로 1.0보다 큼
    
    create_test_pose(&pose, 30.0f, 150.0f);  // 작은 체형
    scale_factor = calculate_pose_scale_factor(&pose);
    assert(scale_factor < 1.0f);  // 작은 체형이므로 1.0보다 작음
    
    printf("  ✓ 포즈 스케일 팩터 계산 테스트 통과\n");
}

void test_calculate_pose_center() {
    printf("테스트: calculate_pose_center\n");
    
    PoseData pose;
    create_test_pose(&pose, 40.0f, 170.0f);
    
    Point3D center = calculate_pose_center(&pose);
    
    // 중심점이 합리적인 범위에 있는지 확인
    assert(fabsf(center.x) < 100.0f);
    assert(fabsf(center.y) < 100.0f);
    assert(fabsf(center.z) < 100.0f);
    
    printf("  ✓ 포즈 중심점 계산 테스트 통과\n");
}

void test_calibration_error_cases() {
    printf("테스트: 캘리브레이션 에러 케이스\n");
    
    // NULL 포인터 테스트
    int result = segment_calibrate(NULL, NULL);
    assert(result == SEGMENT_ERROR_INVALID_PARAMETER);
    printf("  ✓ NULL 포인터 에러 테스트 통과\n");
    
    // 신뢰도가 낮은 포즈 테스트
    PoseData low_confidence_pose;
    create_test_pose(&low_confidence_pose, 40.0f, 170.0f);
    for (int i = 0; i < JOINT_COUNT; i++) {
        low_confidence_pose.confidence[i] = 0.3f;  // 낮은 신뢰도
    }
    
    CalibrationData calibration;
    result = segment_calibrate(&low_confidence_pose, &calibration);
    assert(result != SEGMENT_OK);  // 실패해야 함 (구체적인 에러 코드는 무관)
    printf("  ✓ 낮은 신뢰도 에러 테스트 통과\n");
}

int main() {
    printf("=== 캘리브레이션 함수 테스트 ===\n\n");
    
    test_segment_calibrate();
    test_segment_validate_calibration();
    test_apply_calibration_to_pose();
    test_calculate_pose_scale_factor();
    test_calculate_pose_center();
    test_calibration_error_cases();
    
    printf("\n=== 모든 테스트 통과! ===\n");
    return 0;
}
