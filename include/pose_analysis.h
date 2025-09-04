/**
 * @file pose_analysis.h
 * @brief 포즈 분석 및 진행도 계산 함수들 선언
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#ifndef POSE_ANALYSIS_H
#define POSE_ANALYSIS_H

#include "segment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 두 포즈 간 유사도 계산
 * @param pose1 첫 번째 포즈
 * @param pose2 두 번째 포즈
 * @return 유사도 (0.0~1.0)
 */
float segment_calculate_similarity(const PoseData* pose1, const PoseData* pose2);

/**
 * @brief 포즈 데이터 유효성 검사
 * @param pose 검사할 포즈 데이터
 * @return true 유효, false 무효
 */
bool segment_validate_pose(const PoseData* pose);

/**
 * @brief 세그먼트 진행도 계산
 * @param current_pose 현재 포즈
 * @param start_pose 시작 키포즈
 * @param end_pose 종료 키포즈
 * @param care_joints 관심 관절 배열
 * @param care_joint_count 관심 관절 개수
 * @return 진행도 (0.0~1.0)
 */
float calculate_segment_progress(const PoseData* current_pose,
                                const PoseData* start_pose,
                                const PoseData* end_pose,
                                const JointType* care_joints,
                                int care_joint_count);

/**
 * @brief 관절별 교정 벡터 계산
 * @param current_pose 현재 포즈
 * @param target_pose 목표 포즈
 * @param care_joints 관심 관절 배열
 * @param care_joint_count 관심 관절 개수
 * @param corrections 교정 벡터를 저장할 배열
 */
void calculate_correction_vectors(const PoseData* current_pose,
                                 const PoseData* target_pose,
                                 const JointType* care_joints,
                                 int care_joint_count,
                                 Point3D corrections[JOINT_COUNT]);

/**
 * @brief 세그먼트 완료 여부 판단
 * @param current_pose 현재 포즈
 * @param end_pose 종료 키포즈
 * @param care_joints 관심 관절 배열
 * @param care_joint_count 관심 관절 개수
 * @param similarity_threshold 유사도 임계값
 * @return true 완료, false 미완료
 */
bool is_segment_completed(const PoseData* current_pose,
                         const PoseData* end_pose,
                         const JointType* care_joints,
                         int care_joint_count,
                         float similarity_threshold);

/**
 * @brief 포즈 정규화 (중심점 맞춤)
 * @param input_pose 입력 포즈
 * @param reference_center 참조 중심점
 * @param normalized_pose 정규화된 포즈를 저장할 구조체
 */
void normalize_pose_center(const PoseData* input_pose,
                          const Point3D* reference_center,
                          PoseData* normalized_pose);

#ifdef __cplusplus
}
#endif

#endif // POSE_ANALYSIS_H
