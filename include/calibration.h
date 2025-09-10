/**
 * @file calibration.h
 * @brief 캘리브레이션 관련 함수들 선언
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "segment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 사용자의 기본 포즈로 개인화 캘리브레이션 수행
 * @param base_pose 사용자가 자연스럽게 서있는 자세의 포즈 데이터
 * @param out_calibration 계산된 캘리브레이션 결과를 저장할 구조체
 * @return SEGMENT_OK 성공, 음수 에러 코드
 */
int segment_calibrate(const PoseData* base_pose, CalibrationData* out_calibration);

/**
 * @brief 두 포즈 간 캘리브레이션 데이터 계산
 * @param user_pose 사용자 포즈
 * @param reference_pose 참조 포즈 (이상적 포즈)
 * @param out_calibration 계산된 캘리브레이션 결과를 저장할 구조체
 * @return SEGMENT_OK 성공, 음수 에러 코드
 */
int segment_calibrate_between_poses(const PoseData* user_pose, const PoseData* reference_pose, CalibrationData* out_calibration);

/**
 * @brief 캘리브레이션 데이터 유효성 검사
 * @param calibration 검사할 캘리브레이션 데이터
 * @return true 유효, false 무효
 */
bool segment_validate_calibration(const CalibrationData* calibration);

/**
 * @brief 캘리브레이션을 포즈에 적용
 * @param original_pose 원본 포즈
 * @param calibration 캘리브레이션 데이터
 * @param calibrated_pose 캘리브레이션된 포즈를 저장할 구조체
 * @return SEGMENT_OK 성공, 음수 에러 코드
 */
int apply_calibration_to_pose(const PoseData* original_pose, 
                             const CalibrationData* calibration, 
                             PoseData* calibrated_pose);

/**
 * @brief 포즈의 스케일 팩터 계산
 * @param pose 포즈 데이터
 * @return 스케일 팩터
 */
float calculate_pose_scale_factor(const PoseData* pose);

/**
 * @brief 포즈의 중심점 계산
 * @param pose 포즈 데이터
 * @return 중심점
 */
Point3D calculate_pose_center(const PoseData* pose);

#ifdef __cplusplus
}
#endif

#endif // CALIBRATION_H
