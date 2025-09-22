#include "include/segment_api.h"
#include <stdio.h>

int main() {
  printf("=== JSON 로드 테스트 시작 ===\n");

  // API 초기화
  int result = segment_api_init();
  if (result != SEGMENT_OK) {
    printf("❌ API 초기화 실패: %s\n", segment_get_error_message(result));
    return -1;
  }
  printf("✅ API 초기화 성공\n");

  // 실제적인 캘리브레이션 포즈 (실제 어깨 너비가 있는 포즈)
  PoseData dummy_pose = {0};
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    dummy_pose.landmarks[i].position.x = 400.0f + i * 10.0f; // 다양한 x 좌표
    dummy_pose.landmarks[i].position.y = 800.0f + i * 5.0f; // 다양한 y 좌표
    dummy_pose.landmarks[i].position.z = 0.0f;
    dummy_pose.landmarks[i].inFrameLikelihood = 0.9f;
  }

  // 어깨 위치를 실제적으로 설정 (어깨 너비가 200픽셀)
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.x = 300.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.y = 900.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].position.z = 0.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_SHOULDER].inFrameLikelihood = 0.95f;

  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position.x = 500.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position.y = 900.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].position.z = 0.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_SHOULDER].inFrameLikelihood = 0.95f;

  // 엉덩이 위치도 설정
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_HIP].position.x = 350.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_HIP].position.y = 1200.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_HIP].position.z = 0.0f;
  dummy_pose.landmarks[POSE_LANDMARK_LEFT_HIP].inFrameLikelihood = 0.95f;

  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].position.x = 450.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].position.y = 1200.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].position.z = 0.0f;
  dummy_pose.landmarks[POSE_LANDMARK_RIGHT_HIP].inFrameLikelihood = 0.95f;

  dummy_pose.timestamp = 1000;

  result = segment_calibrate_user(&dummy_pose);
  if (result != SEGMENT_OK) {
    printf("❌ 사용자 캘리브레이션 실패: %s\n",
           segment_get_error_message(result));
    segment_api_cleanup();
    return -1;
  }
  printf("✅ 사용자 캘리브레이션 성공\n");

  // JSON 파일 로드 테스트
  const char *json_file = "test_valid.json";
  result = segment_load_segment(json_file, 0, 1);

  printf("\n=== 테스트 결과 ===\n");
  if (result == SEGMENT_OK) {
    printf("✅ JSON 로드 성공!\n");
  } else {
    printf("❌ JSON 로드 실패 - 오류 코드: %d\n", result);
    printf("오류 메시지: %s\n", segment_get_error_message(result));
  }

  // 정리
  segment_api_cleanup();
  printf("=== 테스트 완료 ===\n");
  return 0;
}
