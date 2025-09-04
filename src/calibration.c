/**
 * @file calibration.c
 * @brief 캘리브레이션 관련 함수들 구현
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#include "calibration.h"
#include "pose_analysis.h"
#include "math_utils.h"
#include <string.h>
#include <math.h>

// 표준 체형 기준값들 (cm 단위)
#define STANDARD_SHOULDER_WIDTH 40.0f
#define STANDARD_HEIGHT 170.0f
#define MIN_CONFIDENCE_THRESHOLD 0.7f
#define MIN_CALIBRATION_QUALITY 0.6f

int segment_calibrate(const PoseData* base_pose, CalibrationData* out_calibration) {
    if (!base_pose || !out_calibration) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    // 포즈 유효성 검사
    if (!segment_validate_pose(base_pose)) {
        return SEGMENT_ERROR_INVALID_POSE;
    }
    
    // 필수 관절들의 신뢰도 확인
    if (base_pose->confidence[JOINT_LEFT_SHOULDER] < MIN_CONFIDENCE_THRESHOLD ||
        base_pose->confidence[JOINT_RIGHT_SHOULDER] < MIN_CONFIDENCE_THRESHOLD ||
        base_pose->confidence[JOINT_LEFT_HIP] < MIN_CONFIDENCE_THRESHOLD ||
        base_pose->confidence[JOINT_RIGHT_HIP] < MIN_CONFIDENCE_THRESHOLD) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    // 어깨 너비 계산
    float user_shoulder_width = distance_3d(
        &base_pose->joints[JOINT_LEFT_SHOULDER],
        &base_pose->joints[JOINT_RIGHT_SHOULDER]
    );
    
    if (user_shoulder_width <= 0.0f) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    // 스케일 팩터 계산
    float scale_factor = user_shoulder_width / STANDARD_SHOULDER_WIDTH;
    
    // 스케일 팩터가 합리적 범위인지 확인
    if (scale_factor < 0.5f || scale_factor > 2.0f) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    // 중심점 계산 (어깨와 골반 중심)
    Point3D shoulder_center = {
        (base_pose->joints[JOINT_LEFT_SHOULDER].x + base_pose->joints[JOINT_RIGHT_SHOULDER].x) * 0.5f,
        (base_pose->joints[JOINT_LEFT_SHOULDER].y + base_pose->joints[JOINT_RIGHT_SHOULDER].y) * 0.5f,
        (base_pose->joints[JOINT_LEFT_SHOULDER].z + base_pose->joints[JOINT_RIGHT_SHOULDER].z) * 0.5f
    };
    
    Point3D hip_center = {
        (base_pose->joints[JOINT_LEFT_HIP].x + base_pose->joints[JOINT_RIGHT_HIP].x) * 0.5f,
        (base_pose->joints[JOINT_LEFT_HIP].y + base_pose->joints[JOINT_RIGHT_HIP].y) * 0.5f,
        (base_pose->joints[JOINT_LEFT_HIP].z + base_pose->joints[JOINT_RIGHT_HIP].z) * 0.5f
    };
    
    Point3D body_center = {
        (shoulder_center.x + hip_center.x) * 0.5f,
        (shoulder_center.y + hip_center.y) * 0.5f,
        (shoulder_center.z + hip_center.z) * 0.5f
    };
    
    // 캘리브레이션 품질 평가
    float calibration_quality = 1.0f;
    
    // 관절 신뢰도 기반 품질 감소
    for (int i = 0; i < JOINT_COUNT; i++) {
        if (base_pose->confidence[i] < MIN_CONFIDENCE_THRESHOLD) {
            calibration_quality -= 0.05f;  // 신뢰도가 낮은 관절마다 5% 감소
        }
    }
    
    // 대칭성 검사 (어깨와 골반의 높이 차이)
    float shoulder_height_diff = fabsf(base_pose->joints[JOINT_LEFT_SHOULDER].y - 
                                      base_pose->joints[JOINT_RIGHT_SHOULDER].y);
    float hip_height_diff = fabsf(base_pose->joints[JOINT_LEFT_HIP].y - 
                                 base_pose->joints[JOINT_RIGHT_HIP].y);
    
    if (shoulder_height_diff > 5.0f || hip_height_diff > 5.0f) {
        calibration_quality -= 0.2f;  // 대칭성이 나쁘면 20% 감소
    }
    
    calibration_quality = clamp(calibration_quality, 0.0f, 1.0f);
    
    if (calibration_quality < MIN_CALIBRATION_QUALITY) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    // 결과 저장
    out_calibration->scale_factor = scale_factor;
    out_calibration->center_offset = body_center;
    out_calibration->is_calibrated = true;
    out_calibration->calibration_quality = calibration_quality;
    
    return SEGMENT_OK;
}

bool segment_validate_calibration(const CalibrationData* calibration) {
    if (!calibration) {
        return false;
    }
    
    // 기본 플래그 확인
    if (!calibration->is_calibrated) {
        return false;
    }
    
    // 스케일 팩터 범위 확인
    if (calibration->scale_factor < 0.5f || calibration->scale_factor > 2.0f) {
        return false;
    }
    
    // 품질 점수 확인
    if (calibration->calibration_quality < MIN_CALIBRATION_QUALITY) {
        return false;
    }
    
    // 중심점 오프셋이 합리적 범위인지 확인
    float center_magnitude = sqrtf(calibration->center_offset.x * calibration->center_offset.x +
                                  calibration->center_offset.y * calibration->center_offset.y +
                                  calibration->center_offset.z * calibration->center_offset.z);
    
    if (center_magnitude > 1000.0f) {  // 10m 이상은 비합리적
        return false;
    }
    
    return true;
}

int apply_calibration_to_pose(const PoseData* original_pose, 
                             const CalibrationData* calibration, 
                             PoseData* calibrated_pose) {
    if (!original_pose || !calibration || !calibrated_pose) {
        return SEGMENT_ERROR_INVALID_PARAMETER;
    }
    
    if (!segment_validate_calibration(calibration)) {
        return SEGMENT_ERROR_CALIBRATION_FAILED;
    }
    
    // 포즈를 스케일링하고 중심점을 이동
    transform_pose(original_pose, 
                  calibration->scale_factor, 
                  &calibration->center_offset, 
                  calibrated_pose);
    
    return SEGMENT_OK;
}

float calculate_pose_scale_factor(const PoseData* pose) {
    if (!pose) return 1.0f;
    
    // 어깨 너비를 기준으로 스케일 팩터 계산
    if (pose->confidence[JOINT_LEFT_SHOULDER] > MIN_CONFIDENCE_THRESHOLD &&
        pose->confidence[JOINT_RIGHT_SHOULDER] > MIN_CONFIDENCE_THRESHOLD) {
        
        float shoulder_width = distance_3d(
            &pose->joints[JOINT_LEFT_SHOULDER],
            &pose->joints[JOINT_RIGHT_SHOULDER]
        );
        
        return shoulder_width / STANDARD_SHOULDER_WIDTH;
    }
    
    return 1.0f;
}

Point3D calculate_pose_center(const PoseData* pose) {
    if (!pose) {
        Point3D zero = {0.0f, 0.0f, 0.0f};
        return zero;
    }
    
    return calculate_center_point(pose);
}
