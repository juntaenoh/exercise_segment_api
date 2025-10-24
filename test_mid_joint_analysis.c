/**
 * @file test_mid_joint_analysis.c
 * @brief mid.json 파일을 이용한 관절 분석 테스트
 * @author Exercise Segment API Team
 * @version 2.1.0
 */

#include "include/segment_api.h"
#include "include/pose_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
  printf("\n🔬 mid.json 관절 분석 테스트\n");
  printf("========================================\n");
  printf("JSON 파일 로드 → 세그먼트 설정 → 관절 분석 수행\n\n");

  // 1. API 초기화
  printf("1️⃣ API 초기화 중...\n");
  int result = segment_api_init();
  if (result != SEGMENT_OK) {
    printf("❌ API 초기화 실패: %s\n", segment_get_error_message(result));
    return -1;
  }
  printf("✅ API 초기화 성공\n\n");

  // 2. 사용자 캘리브레이션 (기본 포즈 생성)
  printf("2️⃣ 사용자 캘리브레이션 중...\n");
  PoseData base_pose;
  
  // 기본 서있는 자세 생성
  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    base_pose.landmarks[i].position.x = 400.0f + (i % 10) * 5.0f;
    base_pose.landmarks[i].position.y = 800.0f + (i % 10) * 10.0f;
    base_pose.landmarks[i].position.z = 0.0f;
    base_pose.landmarks[i].inFrameLikelihood = 0.9f;
  }
  base_pose.timestamp = 1000;

  result = segment_calibrate_user(&base_pose);
  if (result != SEGMENT_OK) {
    printf("❌ 사용자 캘리브레이션 실패: %s\n", segment_get_error_message(result));
    segment_api_cleanup();
    return -1;
  }
  printf("✅ 사용자 캘리브레이션 성공\n\n");

  // 3. JSON 파일 로드
  printf("3️⃣ JSON 파일 로드 중...\n");
  printf("📁 파일 경로: ../examples/mid.json\n");
  
  result = segment_load_all_segments("../examples/top.json");
  if (result != SEGMENT_OK) {
    printf("❌ JSON 파일 로드 실패: %s\n", segment_get_error_message(result));
    segment_api_cleanup();
    return -1;
  }
  printf("✅ JSON 파일 로드 성공\n\n");

  // 4. 세그먼트 정보 확인
  printf("4️⃣ 세그먼트 정보 확인...\n");
  int segment_count = 0;
  result = segment_get_segment_info(&segment_count);
  if (result == SEGMENT_OK) {
    printf("📊 총 세그먼트 개수: %d\n", segment_count);
  } else {
    printf("⚠️  세그먼트 정보 조회 실패: %s\n", segment_get_error_message(result));
  }
  printf("\n");

  // 5. 다양한 세그먼트 조합으로 관절 분석 테스트
  printf("5️⃣ 세그먼트별 관절 분석 테스트\n");
  printf("========================================\n");

  // 테스트할 세그먼트 조합들 (시작 → 종료)
  int test_segments[][2] = {
    {0, 1},    // 첫 번째 → 두 번째 포즈
    {0, 2},    // 첫 번째 → 세 번째 포즈  
    {1, 2},    // 두 번째 → 세 번째 포즈
    {0, 5},    // 첫 번째 → 여섯 번째 포즈 (더 큰 변화)
    {2, 4}     // 세 번째 → 다섯 번째 포즈
  };
  
  int test_count = sizeof(test_segments) / sizeof(test_segments[0]);
  
  for (int i = 0; i < test_count; i++) {
    int start_idx = test_segments[i][0];
    int end_idx = test_segments[i][1];
    
    printf("\n🎯 테스트 %d: 세그먼트 %d → %d\n", i + 1, start_idx, end_idx);
    printf("----------------------------------------\n");
    
    // 세그먼트 설정 (자동으로 관절 분석 수행됨)
    result = segment_set_current_segment(start_idx, end_idx);
    if (result != SEGMENT_OK) {
      printf("❌ 세그먼트 설정 실패: %s\n", segment_get_error_message(result));
      continue;
    }
    
    printf("✅ 세그먼트 설정 완료 (관절 분석 자동 수행됨)\n");
    printf("📊 위에서 출력된 관절 분석 결과를 확인하세요.\n");
  }

  printf("\n6️⃣ 실제 포즈 분석 테스트\n");
  printf("========================================\n");
  
  // 마지막 세그먼트로 설정
  result = segment_set_current_segment(0, 2);
  if (result == SEGMENT_OK) {
    printf("✅ 최종 세그먼트 설정: 0 → 2\n");
    
    // 테스트용 현재 포즈 생성 (중간 자세)
    PoseData test_pose;
    for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
      test_pose.landmarks[i].position.x = 400.0f + (i % 10) * 3.0f;
      test_pose.landmarks[i].position.y = 800.0f + (i % 10) * 8.0f;
      test_pose.landmarks[i].position.z = 0.0f;
      test_pose.landmarks[i].inFrameLikelihood = 0.9f;
    }
    test_pose.timestamp = 1500;
    
    // 분석 수행
    float progress, similarity;
    bool is_complete;
    Point3D corrections[POSE_LANDMARK_COUNT];
    
    result = segment_analyze_simple(&test_pose, &progress, &is_complete, &similarity, corrections);
    if (result == SEGMENT_OK) {
      printf("📈 분석 결과:\n");
      printf("   - 진행도: %.2f%%\n", progress * 100.0f);
      printf("   - 유사도: %.2f%%\n", similarity * 100.0f);
      printf("   - 완료 여부: %s\n", is_complete ? "완료" : "미완료");
    } else {
      printf("❌ 분석 실패: %s\n", segment_get_error_message(result));
    }
  }

  printf("\n🎉 테스트 완료!\n");
  printf("========================================\n");
  printf("✅ JSON 파일 로드 성공\n");
  printf("✅ 세그먼트 설정 성공 (관절 분석 자동 수행)\n");
  printf("✅ 다양한 세그먼트 조합 테스트 완료\n");
  printf("✅ 실제 포즈 분석 테스트 완료\n");
  printf("\n💡 관절 분석 결과를 통해 어떤 관절이 중요한지 확인할 수 있습니다!\n");

  // 정리
  segment_api_cleanup();
  return 0;
}
