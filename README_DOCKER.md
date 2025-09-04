# Exercise Segment API - Docker 사용 가이드

## 🐳 Docker로 어디서든 사용하기

### 1. Docker 이미지 빌드
```bash
# 프로젝트 루트에서
docker build -t exercise-segment-api .
```

### 2. 컨테이너 실행
```bash
# Python 예제 실행
docker run --rm exercise-segment-api

# 개발 모드 (bash 접속)
docker run -it --rm -v $(pwd):/app exercise-segment-api /bin/bash
```

### 3. Docker Compose 사용
```bash
# 예제 실행
docker-compose up exercise-segment-api

# 개발 환경
docker-compose up exercise-segment-dev
```

## 🌐 Google Colab에서 사용

### Colab 노트북에서:
```python
# 1. Docker 이미지 다운로드 (GitHub에서)
!git clone https://github.com/your-repo/exercise_segment_api.git
!cd exercise_segment_api

# 2. Docker 빌드
!docker build -t exercise-segment-api .

# 3. 컨테이너 실행
!docker run --rm exercise-segment-api
```

## 🚀 AWS/GCP/Azure에서 사용

### 어디서든 동일하게:
```bash
# 1. 소스코드 클론
git clone https://github.com/your-repo/exercise_segment_api.git
cd exercise_segment_api

# 2. Docker 빌드
docker build -t exercise-segment-api .

# 3. 실행
docker run --rm exercise-segment-api
```

## 📦 패키지 배포

### Docker Hub에 업로드:
```bash
# 1. 태그 설정
docker tag exercise-segment-api your-username/exercise-segment-api:latest

# 2. 푸시
docker push your-username/exercise-segment-api:latest
```

### 다른 사람이 사용:
```bash
# 어디서든 실행 가능
docker run --rm your-username/exercise-segment-api:latest
```

## 🔧 개발 환경

### 로컬 개발:
```bash
# 소스코드 마운트하여 개발
docker run -it --rm -v $(pwd):/app exercise-segment-api /bin/bash

# 컨테이너 내에서
cd /app
mkdir build && cd build
cmake .. && make
python3 ../python_example/example.py
```

## ✅ 장점

- **어디서든 동일**: Windows, macOS, Linux 상관없이
- **환경 독립**: 호스트 OS와 상관없이
- **의존성 해결**: 모든 라이브러리가 포함됨
- **버전 관리**: Docker 이미지로 버전 관리
- **배포 간편**: 한 번 빌드하면 어디서든 실행

## 🎯 사용 시나리오

1. **개발**: 로컬에서 Docker로 개발
2. **테스트**: Colab에서 Docker로 테스트
3. **배포**: AWS/GCP에서 Docker로 배포
4. **공유**: Docker Hub로 다른 사람과 공유

**이제 정말로 어디서든 사용할 수 있습니다!** 🎉
