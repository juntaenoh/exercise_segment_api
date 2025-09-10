/**
 * @file math_utils.h
 * @brief 수학 유틸리티 함수들
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "segment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 두 2D 점 사이의 유클리드 거리 계산
 * @param p1 첫 번째 점
 * @param p2 두 번째 점
 * @return 거리 값
 */
float distance_2d(const Point2D* p1, const Point2D* p2);

/**
 * @brief 두 3D 점 사이의 유클리드 거리 계산
 * @param p1 첫 번째 점
 * @param p2 두 번째 점
 * @return 거리 값
 */
float distance_3d(const Point3D* p1, const Point3D* p2);

/**
 * @brief 두 2D 점의 합 계산
 * @param p1 첫 번째 점
 * @param p2 두 번째 점
 * @return 합계 점
 */
Point2D add_points(const Point2D* p1, const Point2D* p2);

/**
 * @brief 두 2D 점의 차 계산
 * @param p1 첫 번째 점
 * @param p2 두 번째 점
 * @return 차이 점
 */
Point2D subtract_points(const Point2D* p1, const Point2D* p2);

/**
 * @brief 2D 점에 스칼라 곱하기
 * @param p 점
 * @param scalar 스칼라 값
 * @return 곱한 결과 점
 */
Point2D multiply_point(const Point2D* p, float scalar);

/**
 * @brief 포즈의 중심점 계산
 * @param pose 포즈 데이터
 * @return 중심점
 */
Point2D calculate_center_point(const PoseData* pose);

/**
 * @brief 포즈의 중심점 계산 (특정 관절들만)
 * @param pose 포즈 데이터
 * @param joints 관절 인덱스 배열
 * @param joint_count 관절 개수
 * @return 중심점
 */
Point2D calculate_center_point_selected(const PoseData* pose, 
                                       const JointType* joints, 
                                       int joint_count);

/**
 * @brief 두 포즈 간 선형 보간
 * @param start 시작 포즈
 * @param end 종료 포즈
 * @param t 보간 계수 (0.0~1.0)
 * @param result 보간 결과를 저장할 포즈
 */
void interpolate_pose(const PoseData* start, 
                     const PoseData* end, 
                     float t, 
                     PoseData* result);

/**
 * @brief 값을 지정된 범위로 제한
 * @param value 제한할 값
 * @param min 최소값
 * @param max 최대값
 * @return 제한된 값
 */
float clamp(float value, float min, float max);

/**
 * @brief 제곱근 계산 (빠른 근사)
 * @param x 입력값
 * @return 제곱근
 */
float fast_sqrt(float x);

/**
 * @brief 포즈를 스케일링
 * @param pose 원본 포즈
 * @param scale_factor 스케일 팩터
 * @param result 스케일된 포즈를 저장할 구조체
 */
void scale_pose(const PoseData* pose, float scale_factor, PoseData* result);

/**
 * @brief 포즈를 이동
 * @param pose 원본 포즈
 * @param offset 이동 오프셋
 * @param result 이동된 포즈를 저장할 구조체
 */
void translate_pose(const PoseData* pose, const Point2D* offset, PoseData* result);

/**
 * @brief 포즈를 스케일링하고 이동
 * @param pose 원본 포즈
 * @param scale_factor 스케일 팩터
 * @param offset 이동 오프셋
 * @param result 변환된 포즈를 저장할 구조체
 */
void transform_pose(const PoseData* pose, 
                   float scale_factor, 
                   const Point2D* offset, 
                   PoseData* result);

#ifdef __cplusplus
}
#endif

#endif // MATH_UTILS_H
