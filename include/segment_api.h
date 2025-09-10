/**
 * @file segment_api.h
 * @brief 운동 세그먼트 분석 API의 메인 인터페이스
 * @author Exercise Segment API Team
 * @version 2.0.0
 *
 * @details
 * 이 헤더 파일은 Exercise Segment Analysis API의 모든 공개 함수를 정의합니다.
 * v2.0.0에서는 기록자(A)와 사용자(B) 역할 분리 및 JSON 기반 워크아웃 관리가
 * 추가되었습니다.
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
 * 내부 메모리 풀 초기화, 전역 상태 변수 초기화, 수학 라이브러리 초기화를
 * 수행합니다. 다른 모든 API 함수를 사용하기 전에 반드시 호출해야 합니다.
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
 * @brief 포즈 데이터 유효성 검사
 * @param pose 검사할 포즈 데이터
 * @return true 유효함, false 유효하지 않음
 */
bool segment_validate_pose(const PoseData *pose);

// MARK: - A 이용자 (기록자) API

/**
 * @brief A 이용자의 기본 포즈로 캘리브레이션 수행
 * @param base_pose A 이용자가 자연스럽게 서있는 자세의 포즈 데이터
 * @return SEGMENT_OK 성공, 음수 에러 코드
 *
 * A의 포즈를 API 내부 이상적 기본 포즈와 비교하여 신체 비율 차이를 계산하고,
 * 내부에 저장합니다. 이후 A가 기록하는 모든 포즈가 이상적 비율로 변환됩니다.
 */
int segment_calibrate_recorder(const PoseData *base_pose);

/**
 * @brief A 이용자가 현재 포즈를 기록하여 JSON 파일에 저장
 * @param current_pose A 이용자의 현재 포즈 데이터
 * @param pose_name 포즈 이름 (예: "standing", "squat_down")
 * @param json_file_path JSON 파일 경로
 * @return SEGMENT_OK 성공, 음수 에러 코드
 *
 * A의 포즈를 이상적 비율로 변환하여 JSON 파일에 저장합니다.
 * segment_calibrate_recorder()가 먼저 호출되어야 합니다.
 */
int segment_record_pose(const PoseData *current_pose, const char *pose_name,
                        const char *json_file_path);

/**
 * @brief 워크아웃 JSON 파일을 완성
 * @param workout_name 워크아웃 이름 (예: "squat", "pushup")
 * @param json_file_path JSON 파일 경로
 * @return SEGMENT_OK 성공, 음수 에러 코드
 *
 * 기록된 모든 포즈를 하나의 워크아웃으로 묶어서 JSON 파일을 완성합니다.
 */
int segment_finalize_workout_json(const char *workout_name,
                                  const char *json_file_path);

// MARK: - B 이용자 (사용자) API

/**
 * @brief B 이용자의 기본 포즈로 캘리브레이션 수행
 * @param base_pose B 이용자가 자연스럽게 서있는 자세의 포즈 데이터
 * @return SEGMENT_OK 성공, 음수 에러 코드
 *
 * B의 포즈를 API 내부 이상적 기본 포즈와 비교하여 신체 비율 차이를 계산하고,
 * 내부에 저장합니다. 이후 B가 사용하는 모든 포즈가 B의 체형에 맞게 변환됩니다.
 */
int segment_calibrate_user(const PoseData *base_pose);

/**
 * @brief JSON 파일에서 세그먼트 로드
 * @param json_file_path JSON 파일 경로
 * @param start_index 시작 포즈 인덱스
 * @param end_index 종료 포즈 인덱스
 * @return SEGMENT_OK 성공, 음수 에러 코드
 *
 * JSON 파일에서 두 개의 인덱스로 포즈를 로드하고, 이상적 비율의 포즈를
 * B의 체형에 맞게 변환하여 내부에 저장합니다.
 * segment_calibrate_user()가 먼저 호출되어야 합니다.
 */
int segment_load_segment(const char *json_file_path, int start_index,
                         int end_index);

/**
 * @brief 실시간 포즈 분석 및 피드백 생성
 * @param current_pose B 이용자의 현재 포즈 데이터
 * @return 분석 결과 구조체
 *
 * B의 현재 포즈를 로드된 세그먼트의 시작→종료 포즈와 비교하여
 * 진행도, 교정 벡터, 완료 여부를 계산합니다.
 */
SegmentOutput segment_analyze(const PoseData *current_pose);

/**
 * @brief 현재 세그먼트의 변환된 종료 포즈 반환
 * @param out_pose 변환된 종료 포즈를 저장할 구조체
 * @return SEGMENT_OK 성공, 음수 에러 코드
 *
 * 현재 로드된 세그먼트의 종료 포즈를 B의 체형에 맞게 변환된 상태로 반환합니다.
 * B가 최종적으로 도달해야 하는 목표 포즈입니다.
 */
int segment_get_transformed_end_pose(PoseData *out_pose);

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
 * @brief 포즈 데이터 유효성 검사
 * @param pose 검사할 포즈 데이터
 * @return true 유효, false 무효
 *
 * NULL 포인터 체크, 필수 관절들의 신뢰도 확인, 좌표값 범위 검사,
 * 타임스탬프 유효성을 검사합니다.
 */
bool segment_validate_pose(const PoseData *pose);

/**
 * @brief 에러 코드에 해당하는 설명 문자열 반환
 * @param error_code 에러 코드
 * @return 에러 설명 문자열 (읽기 전용)
 */
const char *segment_get_error_message(int error_code);

// MARK: - Swift 친화적인 함수들 (v2.0.0)
/**
 * @brief Swift에서 사용하기 편리하도록 설계된 함수들
 *
 * 이 함수들은 Swift의 타입 시스템과 메모리 관리에 최적화되어 있습니다.
 * v2.0.0에서는 새로운 API 구조에 맞게 업데이트되었습니다.
 */

/**
 * @brief Swift에서 사용하기 편한 분석 함수
 * @param current_pose 현재 포즈
 * @param out_progress 진행도 출력 (0.0~1.0)
 * @param out_is_complete 완료 여부 출력
 * @param out_similarity 유사도 출력 (0.0~1.0)
 * @param out_corrections 교정 벡터 배열 출력 (33개 랜드마크)
 * @return SEGMENT_OK 성공, 음수 에러 코드
 *
 * Swift에서 구조체 대신 개별 값으로 결과를 받을 수 있도록 개선된 함수
 */
int segment_analyze_simple(const PoseData *current_pose, float *out_progress,
                           bool *out_is_complete, float *out_similarity,
                           Point3D *out_corrections);

/**
 * @brief 포즈 데이터를 Swift에서 안전하게 생성하는 헬퍼 함수
 * @param landmarks 33개 랜드마크 좌표 배열
 * @param out_pose 생성된 포즈 데이터를 저장할 구조체
 * @return SEGMENT_OK 성공, 음수 에러 코드
 */
int segment_create_pose_data(const PoseLandmark *landmarks, PoseData *out_pose);

// MARK: - Swift 호환성을 위한 함수들

/**
 * @brief Swift에서 사용하기 편한 캘리브레이션 함수
 * @param base_pose 기본 포즈 데이터
 * @return SEGMENT_OK 성공, 음수 에러 코드
 */
int segment_calibrate_recorder_swift(const PoseData *base_pose);

/**
 * @brief Swift에서 사용하기 편한 포즈 기록 함수
 * @param current_pose 현재 포즈 데이터
 * @param pose_name 포즈 이름
 * @param json_file_path JSON 파일 경로
 * @return SEGMENT_OK 성공, 음수 에러 코드
 */
int segment_record_pose_swift(const PoseData *current_pose,
                              const char *pose_name,
                              const char *json_file_path);

#ifdef __cplusplus
}
#endif

#endif // SEGMENT_API_H
