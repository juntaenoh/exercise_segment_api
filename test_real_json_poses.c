#include "include/segment_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// JSON에서 특정 인덱스의 포즈를 직접 읽어오는 헬퍼 함수
int load_specific_pose_from_json(const char *json_path, int pose_index,
                                 PoseData *out_pose) {
  FILE *file = fopen(json_path, "r");
  if (!file)
    return -1;

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = malloc(size + 1);
  fread(buffer, 1, size, file);
  buffer[size] = '\0';
  fclose(file);

  // poses 배열 찾기
  char *poses_start = strstr(buffer, "\"poses\"");
  if (!poses_start) {
    free(buffer);
    return -1;
  }

  char *array_start = strchr(poses_start, '[');

  // 원하는 인덱스까지 이동
  int current_index = 0;
  char *pos = array_start + 1;

  while (current_index < pose_index && *pos) {
    if (*pos == '{') {
      int brace_count = 1;
      pos++;
      while (*pos && brace_count > 0) {
        if (*pos == '{')
          brace_count++;
        if (*pos == '}')
          brace_count--;
        pos++;
      }
      current_index++;
    } else {
      pos++;
    }
  }

  // 해당 포즈의 landmarks 파싱
  char *landmarks_start = strstr(pos, "\"landmarks\"");
  if (!landmarks_start) {
    free(buffer);
    return -1;
  }

  char *landmarks_array = strchr(landmarks_start, '[');
  pos = landmarks_array + 1;

  for (int i = 0; i < POSE_LANDMARK_COUNT; i++) {
    // "position" 찾기
    char *position_start = strstr(pos, "\"position\"");
    if (!position_start)
      break;

    // x, y, z 파싱
    char *x_start = strstr(position_start, "\"x\"");
    char *y_start = strstr(position_start, "\"y\"");
    char *z_start = strstr(position_start, "\"z\"");

    if (x_start && y_start && z_start) {
      sscanf(x_start + 4, ": %f", &out_pose->landmarks[i].position.x);
      sscanf(y_start + 4, ": %f", &out_pose->landmarks[i].position.y);
      sscanf(z_start + 4, ": %f", &out_pose->landmarks[i].position.z);
    }

    // confidence 파싱
    char *conf_start = strstr(position_start, "\"confidence\"");
    if (conf_start) {
      sscanf(conf_start + 13, ": %f",
             &out_pose->landmarks[i].inFrameLikelihood);
    }

    pos = conf_start + 20;
  }

  free(buffer);
  out_pose->timestamp = 0;
  return 0;
}

int main() {
  printf("\n");
  printf("================================================\n");
  printf("🧪 mid.json 실제 포즈로 진행도 테스트\n");
  printf("================================================\n\n");

  // 1. API 초기화
  segment_api_init();

  // 2. 첫 번째 포즈로 캘리브레이션
  printf("📋 JSON에서 포즈 0번 로드 중...\n");
  PoseData pose_0;
  if (load_specific_pose_from_json("build/mid.json", 0, &pose_0) != 0) {
    printf("❌ 포즈 로드 실패\n");
    return 1;
  }
  printf("✅ 포즈 0번 로드 완료\n");

  printf("📋 포즈 0번으로 캘리브레이션 중...\n");
  segment_calibrate_user(&pose_0);
  printf("✅ 캘리브레이션 완료\n\n");

  // 3. 모든 세그먼트 로드
  printf("📂 build/mid.json 전체 로드 중...\n");
  segment_load_all_segments("build/mid.json");
  printf("✅ 세그먼트 로드 완료\n\n");

  // 4. 세그먼트 0→1 설정
  printf("================================================\n");
  printf("📊 테스트: 세그먼트 0→1\n");
  printf("================================================\n");
  segment_set_current_segment(0, 1);

  float progress, similarity;
  bool is_complete;
  Point3D corrections[POSE_LANDMARK_COUNT];

  // 포즈 0번으로 분석 (시작 포즈 = 현재 포즈)
  printf("\n🔹 현재 포즈 = 0번 (시작 포즈)\n");
  segment_analyze_simple(&pose_0, &progress, &is_complete, &similarity,
                         corrections);
  printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
         similarity * 100, is_complete ? "✅" : "❌");

  // 포즈 1번 로드 후 분석 (종료 포즈 = 현재 포즈)
  printf("\n🔹 현재 포즈 = 1번 (종료 포즈)\n");
  PoseData pose_1;
  if (load_specific_pose_from_json("build/mid.json", 1, &pose_1) == 0) {
    segment_analyze_simple(&pose_1, &progress, &is_complete, &similarity,
                           corrections);
    printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
           similarity * 100, is_complete ? "✅" : "❌");
  }

  // 5. 세그먼트 0→2 설정
  printf("\n================================================\n");
  printf("📊 테스트: 세그먼트 0→2\n");
  printf("================================================\n");
  segment_set_current_segment(0, 2);

  printf("\n🔹 현재 포즈 = 0번 (시작 포즈)\n");
  segment_analyze_simple(&pose_0, &progress, &is_complete, &similarity,
                         corrections);
  printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
         similarity * 100, is_complete ? "✅" : "❌");

  printf("\n🔹 현재 포즈 = 1번 (중간)\n");
  segment_analyze_simple(&pose_1, &progress, &is_complete, &similarity,
                         corrections);
  printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
         similarity * 100, is_complete ? "✅" : "❌");

  printf("\n🔹 현재 포즈 = 2번 (종료 포즈)\n");
  PoseData pose_2;
  if (load_specific_pose_from_json("build/mid.json", 2, &pose_2) == 0) {
    segment_analyze_simple(&pose_2, &progress, &is_complete, &similarity,
                           corrections);
    printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
           similarity * 100, is_complete ? "✅" : "❌");
  }

  // 6. 세그먼트 0→3 설정
  printf("\n================================================\n");
  printf("📊 테스트: 세그먼트 0→3\n");
  printf("================================================\n");
  segment_set_current_segment(0, 3);

  printf("\n🔹 현재 포즈 = 0번 (시작 포즈)\n");
  segment_analyze_simple(&pose_0, &progress, &is_complete, &similarity,
                         corrections);
  printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
         similarity * 100, is_complete ? "✅" : "❌");

  printf("\n🔹 현재 포즈 = 1번\n");
  segment_analyze_simple(&pose_1, &progress, &is_complete, &similarity,
                         corrections);
  printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
         similarity * 100, is_complete ? "✅" : "❌");

  printf("\n🔹 현재 포즈 = 2번\n");
  segment_analyze_simple(&pose_2, &progress, &is_complete, &similarity,
                         corrections);
  printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
         similarity * 100, is_complete ? "✅" : "❌");

  printf("\n🔹 현재 포즈 = 3번 (종료 포즈)\n");
  PoseData pose_3;
  if (load_specific_pose_from_json("build/mid.json", 3, &pose_3) == 0) {
    segment_analyze_simple(&pose_3, &progress, &is_complete, &similarity,
                           corrections);
    printf("   진행도: %.1f%%, 유사도: %.1f%%, 완료: %s\n", progress * 100,
           similarity * 100, is_complete ? "✅" : "❌");
  }

  printf("\n================================================\n");
  printf("✅ 테스트 완료\n");
  printf("================================================\n\n");

  return 0;
}
