#include "include/segment_api.h"
#include <stdio.h>

int main() {
  printf("=== 테스트 JSON 생성 시작 ===\n");

  // API 초기화
  int result = segment_api_init();
  if (result != SEGMENT_OK) {
    printf("❌ API 초기화 실패: %s\n", segment_get_error_message(result));
    return -1;
  }

  // 기록자로 캘리브레이션 (실제적인 어깨 너비 설정)
  PoseData dummy_pose = {0};
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    dummy_pose.landmarks[i].position.x = 400.0f + i * 5.0f;
    dummy_pose.landmarks[i].position.y = 800.0f + i * 2.0f;
    dummy_pose.landmarks[i].position.z = 0.0f;
    dummy_pose.landmarks[i].inFrameLikelihood = 0.9f;
  }

  // 어깨 위치를 실제적으로 설정 (어깨 너비가 200픽셀)
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.x = 300.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.y = 900.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].inFrameLikelihood = 0.95f;

  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position.x = 500.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position.y = 900.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].inFrameLikelihood = 0.95f;

  dummy_pose.timestamp = 1000;

  result = segment_calibrate_recorder(&dummy_pose);
  if (result != SEGMENT_OK) {
    printf("❌ 기록자 캘리브레이션 실패: %s\n",
           segment_get_error_message(result));
    segment_api_cleanup();
    return -1;
  }
  printf("✅ 기록자 캘리브레이션 성공\n");

  // 첫 번째 포즈 기록 (서기)
  PoseData pose1 = dummy_pose;
  pose1.timestamp = 1000;
  result = segment_record_pose(&pose1, "standing", "test_valid.json");
  if (result != SEGMENT_OK) {
    printf("❌ 첫 번째 포즈 기록 실패: %s\n",
           segment_get_error_message(result));
    segment_api_cleanup();
    return -1;
  }
  printf("✅ 첫 번째 포즈 기록 성공\n");

  // 두 번째 포즈 기록 (스쿼트)
  PoseData pose2 = dummy_pose;
  // 스쿼트 자세로 변경 (무릎을 구부린 자세)
  pose2.landmarks[POSE_LANDMARK_LEFT_KNEE].position.y += 100.0f;
  pose2.landmarks[POSE_LANDMARK_RIGHT_KNEE].position.y += 100.0f;
  pose2.timestamp = 2000;
  result = segment_record_pose(&pose2, "squat_down", "test_valid.json");
  if (result != SEGMENT_OK) {
    printf("❌ 두 번째 포즈 기록 실패: %s\n",
           segment_get_error_message(result));
    segment_api_cleanup();
    return -1;
  }
  printf("✅ 두 번째 포즈 기록 성공\n");

  // JSON 파일 완성
  result = segment_finalize_workout_json("test_workout", "test_valid.json");
  if (result != SEGMENT_OK) {
    printf("❌ JSON 파일 완성 실패: %s\n", segment_get_error_message(result));
    segment_api_cleanup();
    return -1;
  }
  printf("✅ JSON 파일 완성 성공\n");

  // 정리
  segment_api_cleanup();
  printf("=== 테스트 JSON 생성 완료 ===\n");
  return 0;
}
