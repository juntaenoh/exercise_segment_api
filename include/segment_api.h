/**
 * @file segment_api.h
 * @brief 운동 세그먼트 분석 API의 메인 인터페이스
 * @author Exercise Segment API Team
 * @version 1.0.0
 */

#ifndef SEGMENT_API_H
#define SEGMENT_API_H

#include "segment_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief API 시스템 초기화
 * @return SEGMENT_OK 성공, 음수 에러 코드
 * 
 * 내부 메모리 풀 초기화, 전역 상태 변수 초기화, 수학 라이브러리 초기화를 수행합니다.
 * 다른 모든 API 함수를 사용하기 전에 반드시 호출해야 합니다.
 */
int segment_api_init(void);

/**
 * @brief API 시스템 정리 및 메모리 해제
 * 
 * 모든 할당된 메모리 해제, 세그먼트 자동 해제, 정리 완료 상태로 전환합니다.
 * 프로그램 종료 전에 반드시 호출해야 합니다.
 */
void segment_api_cleanup(void);

/**
 * @brief 사용자의 기본 포즈로 개인화 캘리브레이션 수행
 * @param base_pose 사용자가 자연스럽게 서있는 자세의 포즈 데이터
 * @param out_calibration 계산된 캘리브레이션 결과를 저장할 구조체
 * @return SEGMENT_OK 성공, 음수 에러 코드
 * 
 * 포즈 유효성 검사, 주요 관절 간 거리 측정, 표준 크기 대비 스케일 팩터 계산,
 * 중심점 오프셋 계산, 캘리브레이션 품질 평가를 수행합니다.
 */
int segment_calibrate(const PoseData* base_pose, CalibrationData* out_calibration);

/**
 * @brief 캘리브레이션 데이터 유효성 검사
 * @param calibration 검사할 캘리브레이션 데이터
 * @return true 유효, false 무효
 * 
 * is_calibrated 플래그, scale_factor 범위, calibration_quality 임계값을 검사합니다.
 */
bool segment_validate_calibration(const CalibrationData* calibration);

/**
 * @brief 운동 세그먼트 생성 및 설정
 * @param start_keypose 시작 키포즈 (개발자가 미리 준비)
 * @param end_keypose 종료 키포즈 (개발자가 미리 준비)
 * @param calibration 사용자 캘리브레이션 데이터
 * @param care_joints 이 운동에서 중요한 관절들의 배열
 * @param care_joint_count care_joints 배열 크기
 * @return SEGMENT_OK 성공, 음수 에러 코드
 * 
 * 입력 매개변수 유효성 검사, 키포즈들을 캘리브레이션에 따라 개인화,
 * Care 영역 설정 저장, 내부 세그먼트 상태 초기화를 수행합니다.
 */
int segment_create(const PoseData* start_keypose, 
                   const PoseData* end_keypose,
                   const CalibrationData* calibration,
                   const JointType* care_joints,
                   int care_joint_count);

/**
 * @brief 실시간 포즈 분석 및 피드백 생성
 * @param input 현재 포즈 데이터
 * @return 분석 결과 구조체
 * 
 * 입력 포즈를 캘리브레이션에 따라 정규화, 진행도 계산, 목표 키포즈 계산,
 * 각 관절별 교정 벡터 계산, 완료 여부 판단을 수행합니다.
 */
SegmentOutput segment_analyze(const SegmentInput* input);

/**
 * @brief 현재 세그먼트를 초기 상태로 리셋
 * @return SEGMENT_OK 성공, 음수 에러 코드
 * 
 * 진행 상태만 초기화하고 설정은 유지합니다.
 */
int segment_reset(void);

/**
 * @brief 현재 세그먼트 해제
 * 
 * 세그먼트 관련 모든 메모리를 해제합니다.
 */
void segment_destroy(void);

/**
 * @brief 두 포즈 간 유사도 계산
 * @param pose1 첫 번째 포즈
 * @param pose2 두 번째 포즈
 * @return 유사도 (0.0~1.0)
 * 
 * 평균 관절 거리를 기반으로 유사도를 계산합니다.
 */
float segment_calculate_similarity(const PoseData* pose1, const PoseData* pose2);

/**
 * @brief 포즈 데이터 유효성 검사
 * @param pose 검사할 포즈 데이터
 * @return true 유효, false 무효
 * 
 * NULL 포인터 체크, 필수 관절들의 신뢰도 확인, 좌표값 범위 검사,
 * 타임스탬프 유효성을 검사합니다.
 */
bool segment_validate_pose(const PoseData* pose);

/**
 * @brief 에러 코드에 해당하는 설명 문자열 반환
 * @param error_code 에러 코드
 * @return 에러 설명 문자열 (읽기 전용)
 */
const char* segment_get_error_message(int error_code);

#ifdef __cplusplus
}
#endif

#endif // SEGMENT_API_H
