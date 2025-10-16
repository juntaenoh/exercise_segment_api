#include "include/segment_api.h"
#include <stdio.h>
#include <stdlib.h>

void print_separator() {
  printf("================================================\n");
}

int main() {
  printf("\n");
  print_separator();
  printf("🧪 mid.json 진행도 테스트\n");
  print_separator();
  printf("\n");

  // 1. API 초기화
  int result = segment_api_init();
  if (result != SEGMENT_OK) {
    printf("❌ API 초기화 실패\n");
    return 1;
  }
  printf("✅ API 초기화 완료\n\n");

  // 2. 사용자 캘리브레이션 (첫 번째 포즈 사용)
  printf("📋 사용자 캘리브레이션 시작...\n");

  // 간단한 기본 포즈 생성
  PoseData base_pose;
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    base_pose.landmarks[i].position.x = 400;
    base_pose.landmarks[i].position.y = 900 + i * 30;
    base_pose.landmarks[i].position.z = -200;
    base_pose.landmarks[i].inFrameLikelihood = 0.99f;
  }
  base_pose.timestamp = 0;

  result = segment_calibrate_user(&base_pose);
  if (result != SEGMENT_OK) {
    printf("❌ 캘리브레이션 실패\n");
    return 1;
  }
  printf("✅ 캘리브레이션 완료\n\n");

  // 3. JSON에서 모든 세그먼트 로드
  printf("📂 build/mid.json 로드 중...\n");
  result = segment_load_all_segments("build/mid.json");
  if (result != SEGMENT_OK) {
    printf("❌ JSON 로드 실패: 에러 코드 %d\n", result);
    return 1;
  }
  printf("✅ JSON 로드 완료\n\n");

  // 4. 각 세그먼트별 테스트
  float progress, similarity;
  bool is_complete;
  Point3D corrections[POSE_LANDMARK_COUNT];

  // 테스트 시나리오: 0→1 세그먼트
  print_separator();
  printf("📊 테스트 1: 세그먼트 0→1\n");
  print_separator();
  result = segment_set_current_segment(0, 1);
  if (result != SEGMENT_OK) {
    printf("❌ 세그먼트 설정 실패\n");
    return 1;
  }

  // 0번 포즈 (시작 포즈)를 현재 포즈로 테스트
  PoseData test_pose_0 = base_pose;
  result = segment_analyze_simple(&test_pose_0, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 0번 포즈 (시작) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  // 중간 포즈 시뮬레이션
  PoseData test_pose_mid = base_pose;
  test_pose_mid.landmarks[POSE_LANDMARK_LEFT_WRIST].position.y -= 50;
  test_pose_mid.landmarks[POSE_LANDMARK_RIGHT_WRIST].position.y -= 50;
  result = segment_analyze_simple(&test_pose_mid, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 중간 포즈 (50px 이동) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: "
         "%s\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  // 1번 포즈 (종료 포즈)를 현재 포즈로 테스트
  PoseData test_pose_1 = base_pose;
  test_pose_1.landmarks[POSE_LANDMARK_LEFT_WRIST].position.y -= 100;
  test_pose_1.landmarks[POSE_LANDMARK_RIGHT_WRIST].position.y -= 100;
  result = segment_analyze_simple(&test_pose_1, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 1번 포즈 (종료) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  // 테스트 시나리오: 0→2 세그먼트
  print_separator();
  printf("📊 테스트 2: 세그먼트 0→2\n");
  print_separator();
  result = segment_set_current_segment(0, 2);
  if (result != SEGMENT_OK) {
    printf("❌ 세그먼트 설정 실패\n");
    return 1;
  }

  result = segment_analyze_simple(&test_pose_0, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 0번 포즈 (시작) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  PoseData test_pose_mid2 = base_pose;
  test_pose_mid2.landmarks[POSE_LANDMARK_LEFT_WRIST].position.y -= 150;
  test_pose_mid2.landmarks[POSE_LANDMARK_RIGHT_WRIST].position.y -= 150;
  test_pose_mid2.landmarks[POSE_LANDMARK_LEFT_ELBOW].position.y -= 75;
  test_pose_mid2.landmarks[POSE_LANDMARK_RIGHT_ELBOW].position.y -= 75;
  result = segment_analyze_simple(&test_pose_mid2, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 중간 포즈 (150px 이동) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: "
         "%s\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  PoseData test_pose_2 = base_pose;
  test_pose_2.landmarks[POSE_LANDMARK_LEFT_WRIST].position.y -= 300;
  test_pose_2.landmarks[POSE_LANDMARK_RIGHT_WRIST].position.y -= 300;
  test_pose_2.landmarks[POSE_LANDMARK_LEFT_ELBOW].position.y -= 150;
  test_pose_2.landmarks[POSE_LANDMARK_RIGHT_ELBOW].position.y -= 150;
  result = segment_analyze_simple(&test_pose_2, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 2번 포즈 (종료) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  // 테스트 시나리오: 0→3 세그먼트
  print_separator();
  printf("📊 테스트 3: 세그먼트 0→3\n");
  print_separator();
  result = segment_set_current_segment(0, 3);
  if (result != SEGMENT_OK) {
    printf("❌ 세그먼트 설정 실패\n");
    return 1;
  }

  result = segment_analyze_simple(&test_pose_0, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 0번 포즈 (시작) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  PoseData test_pose_mid3 = base_pose;
  test_pose_mid3.landmarks[POSE_LANDMARK_LEFT_WRIST].position.y -= 200;
  test_pose_mid3.landmarks[POSE_LANDMARK_RIGHT_WRIST].position.y -= 200;
  test_pose_mid3.landmarks[POSE_LANDMARK_LEFT_ELBOW].position.y -= 100;
  test_pose_mid3.landmarks[POSE_LANDMARK_RIGHT_ELBOW].position.y -= 100;
  result = segment_analyze_simple(&test_pose_mid3, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 중간 포즈 (200px 이동) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: "
         "%s\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  PoseData test_pose_3 = base_pose;
  test_pose_3.landmarks[POSE_LANDMARK_LEFT_WRIST].position.y -= 400;
  test_pose_3.landmarks[POSE_LANDMARK_RIGHT_WRIST].position.y -= 400;
  test_pose_3.landmarks[POSE_LANDMARK_LEFT_ELBOW].position.y -= 200;
  test_pose_3.landmarks[POSE_LANDMARK_RIGHT_ELBOW].position.y -= 200;
  result = segment_analyze_simple(&test_pose_3, &progress, &is_complete,
                                  &similarity, corrections);
  printf("현재: 3번 포즈 (종료) → 진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n\n",
         progress * 100, similarity * 100, is_complete ? "✅" : "❌");

  print_separator();
  printf("✅ 모든 테스트 완료\n");
  print_separator();
  printf("\n");

  return 0;
}
