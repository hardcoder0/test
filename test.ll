; ModuleID = 'Epic pekoscript app'
source_filename = "Epic pekoscript app"
target triple = "x86_64-pc-linux-gnu"

@0 = private unnamed_addr constant [9 x i8] c"Prestons\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"Dad\00", align 1
@2 = private unnamed_addr constant [4 x i8] c"Mom\00", align 1
@3 = private unnamed_addr constant [5 x i8] c"jamo\00", align 1
@4 = private unnamed_addr constant [9 x i8] c"Prestons\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"Dad\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"Mom\00", align 1
@7 = private unnamed_addr constant [5 x i8] c"jamo\00", align 1

declare double @printnum(double, ...)

declare i8* @input(i8*, ...)

declare double @inputnum(i8*, ...)

declare double @printstr(i8*, ...)

declare i8* @addstr(i8*, i8*, ...)

declare double @cmpstr(i8*, i8*, ...)

declare i8* @mulstr(i8*, double, ...)

declare double @modnum(double, double, ...)

define void @main() {
entry:
  %0 = alloca i8**, align 8
  %malloccall = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %1 = bitcast i8* %malloccall to i8**
  store i8** %1, i8*** %0, align 8
  %2 = load i8**, i8*** %0, align 8
  %3 = getelementptr i8*, i8** %2, i64 0
  %4 = alloca i8***, align 8
  %malloccall1 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %5 = bitcast i8* %malloccall1 to i8***
  store i8*** %5, i8**** %4, align 8
  %6 = load i8***, i8**** %4, align 8
  %7 = getelementptr i8**, i8*** %6, i64 0
  %8 = load i8**, i8*** %0, align 8
  store i8** %8, i8*** %7, align 8
  %9 = alloca i8****, align 8
  %malloccall2 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %10 = bitcast i8* %malloccall2 to i8****
  store i8**** %10, i8***** %9, align 8
  %11 = load i8****, i8***** %9, align 8
  %12 = getelementptr i8***, i8**** %11, i64 0
  %13 = load i8***, i8**** %4, align 8
  store i8*** %13, i8**** %12, align 8
  %14 = alloca i8**, align 8
  %malloccall3 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %15 = bitcast i8* %malloccall3 to i8**
  store i8** %15, i8*** %14, align 8
  %16 = load i8**, i8*** %14, align 8
  %17 = getelementptr i8*, i8** %16, i64 0
  %18 = load i8**, i8*** %14, align 8
  %19 = getelementptr i8*, i8** %18, i64 0
  %20 = load i8*, i8** %19, align 8
  %21 = alloca i8**, align 8
  %22 = load i8**, i8*** %14, align 8
  store i8** %22, i8*** %21, align 8
  %malloccall4 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %23 = bitcast i8* %malloccall4 to i8**
  store i8** %23, i8*** %14, align 8
  %24 = load i8**, i8*** %21, align 8
  store i8** %24, i8*** %14, align 8
  %25 = load i8**, i8*** %14, align 8
  %26 = getelementptr i8*, i8** %25, i64 0
  store i8* getelementptr inbounds ([9 x i8], [9 x i8]* @0, i32 0, i32 0), i8** %26, align 8
  %27 = load i8**, i8*** %14, align 8
  %28 = getelementptr i8*, i8** %27, i64 0
  %29 = load i8*, i8** %28, align 8
  %30 = alloca i8**, align 8
  %31 = load i8**, i8*** %14, align 8
  store i8** %31, i8*** %30, align 8
  %malloccall5 = tail call i8* @malloc(i64 mul (i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64), i64 2))
  %32 = bitcast i8* %malloccall5 to i8**
  store i8** %32, i8*** %14, align 8
  %33 = load i8**, i8*** %30, align 8
  store i8** %33, i8*** %14, align 8
  %34 = load i8**, i8*** %14, align 8
  %35 = getelementptr i8*, i8** %34, i64 1
  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @1, i32 0, i32 0), i8** %35, align 8
  %36 = load i8**, i8*** %14, align 8
  %37 = getelementptr i8*, i8** %36, i64 0
  %38 = load i8*, i8** %37, align 8
  %39 = alloca i8**, align 8
  %40 = load i8**, i8*** %14, align 8
  store i8** %40, i8*** %39, align 8
  %malloccall6 = tail call i8* @malloc(i64 mul (i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64), i64 3))
  %41 = bitcast i8* %malloccall6 to i8**
  store i8** %41, i8*** %14, align 8
  %42 = load i8**, i8*** %39, align 8
  store i8** %42, i8*** %14, align 8
  %43 = load i8**, i8*** %14, align 8
  %44 = getelementptr i8*, i8** %43, i64 2
  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @2, i32 0, i32 0), i8** %44, align 8
  %45 = load i8**, i8*** %14, align 8
  %46 = alloca i8***, align 8
  %malloccall7 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %47 = bitcast i8* %malloccall7 to i8***
  store i8*** %47, i8**** %46, align 8
  %48 = load i8***, i8**** %46, align 8
  %49 = getelementptr i8**, i8*** %48, i64 0
  %50 = load i8***, i8**** %46, align 8
  %51 = getelementptr i8**, i8*** %50, i64 0
  %52 = load i8**, i8*** %51, align 8
  %53 = alloca i8***, align 8
  %54 = load i8***, i8**** %46, align 8
  store i8*** %54, i8**** %53, align 8
  %malloccall8 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %55 = bitcast i8* %malloccall8 to i8***
  store i8*** %55, i8**** %46, align 8
  %56 = load i8***, i8**** %53, align 8
  store i8*** %56, i8**** %46, align 8
  %57 = load i8***, i8**** %46, align 8
  %58 = getelementptr i8**, i8*** %57, i64 0
  store i8** %45, i8*** %58, align 8
  %59 = alloca i8**, align 8
  %malloccall9 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %60 = bitcast i8* %malloccall9 to i8**
  store i8** %60, i8*** %59, align 8
  %61 = load i8**, i8*** %59, align 8
  %62 = getelementptr i8*, i8** %61, i64 0
  %63 = load i8**, i8*** %59, align 8
  %64 = getelementptr i8*, i8** %63, i64 0
  %65 = load i8*, i8** %64, align 8
  %66 = alloca i8**, align 8
  %67 = load i8**, i8*** %59, align 8
  store i8** %67, i8*** %66, align 8
  %malloccall10 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %68 = bitcast i8* %malloccall10 to i8**
  store i8** %68, i8*** %59, align 8
  %69 = load i8**, i8*** %66, align 8
  store i8** %69, i8*** %59, align 8
  %70 = load i8**, i8*** %59, align 8
  %71 = getelementptr i8*, i8** %70, i64 0
  store i8* getelementptr inbounds ([5 x i8], [5 x i8]* @3, i32 0, i32 0), i8** %71, align 8
  %72 = load i8**, i8*** %59, align 8
  %73 = load i8***, i8**** %46, align 8
  %74 = getelementptr i8**, i8*** %73, i64 0
  %75 = load i8**, i8*** %74, align 8
  %76 = alloca i8***, align 8
  %77 = load i8***, i8**** %46, align 8
  store i8*** %77, i8**** %76, align 8
  %malloccall11 = tail call i8* @malloc(i64 mul (i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64), i64 2))
  %78 = bitcast i8* %malloccall11 to i8***
  store i8*** %78, i8**** %46, align 8
  %79 = load i8***, i8**** %76, align 8
  store i8*** %79, i8**** %46, align 8
  %80 = load i8***, i8**** %46, align 8
  %81 = getelementptr i8**, i8*** %80, i64 1
  store i8** %72, i8*** %81, align 8
  %82 = load i8***, i8**** %46, align 8
  %83 = alloca i8**, align 8
  %malloccall12 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %84 = bitcast i8* %malloccall12 to i8**
  store i8** %84, i8*** %83, align 8
  %85 = load i8**, i8*** %83, align 8
  %86 = getelementptr i8*, i8** %85, i64 0
  %87 = load i8**, i8*** %83, align 8
  %88 = getelementptr i8*, i8** %87, i64 0
  %89 = load i8*, i8** %88, align 8
  %90 = alloca i8**, align 8
  %91 = load i8**, i8*** %83, align 8
  store i8** %91, i8*** %90, align 8
  %malloccall13 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %92 = bitcast i8* %malloccall13 to i8**
  store i8** %92, i8*** %83, align 8
  %93 = load i8**, i8*** %90, align 8
  store i8** %93, i8*** %83, align 8
  %94 = load i8**, i8*** %83, align 8
  %95 = getelementptr i8*, i8** %94, i64 0
  store i8* getelementptr inbounds ([9 x i8], [9 x i8]* @4, i32 0, i32 0), i8** %95, align 8
  %96 = load i8**, i8*** %83, align 8
  %97 = getelementptr i8*, i8** %96, i64 0
  %98 = load i8*, i8** %97, align 8
  %99 = alloca i8**, align 8
  %100 = load i8**, i8*** %83, align 8
  store i8** %100, i8*** %99, align 8
  %malloccall14 = tail call i8* @malloc(i64 mul (i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64), i64 2))
  %101 = bitcast i8* %malloccall14 to i8**
  store i8** %101, i8*** %83, align 8
  %102 = load i8**, i8*** %99, align 8
  store i8** %102, i8*** %83, align 8
  %103 = load i8**, i8*** %83, align 8
  %104 = getelementptr i8*, i8** %103, i64 1
  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @5, i32 0, i32 0), i8** %104, align 8
  %105 = load i8**, i8*** %83, align 8
  %106 = getelementptr i8*, i8** %105, i64 0
  %107 = load i8*, i8** %106, align 8
  %108 = alloca i8**, align 8
  %109 = load i8**, i8*** %83, align 8
  store i8** %109, i8*** %108, align 8
  %malloccall15 = tail call i8* @malloc(i64 mul (i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64), i64 3))
  %110 = bitcast i8* %malloccall15 to i8**
  store i8** %110, i8*** %83, align 8
  %111 = load i8**, i8*** %108, align 8
  store i8** %111, i8*** %83, align 8
  %112 = load i8**, i8*** %83, align 8
  %113 = getelementptr i8*, i8** %112, i64 2
  store i8* getelementptr inbounds ([4 x i8], [4 x i8]* @6, i32 0, i32 0), i8** %113, align 8
  %114 = load i8**, i8*** %83, align 8
  %115 = load i8****, i8***** %9, align 8
  %116 = getelementptr i8***, i8**** %115, i64 0
  %117 = load i8***, i8**** %116, align 8
  %118 = alloca i8****, align 8
  %119 = load i8****, i8***** %9, align 8
  store i8**** %119, i8***** %118, align 8
  %malloccall16 = tail call i8* @malloc(i64 0)
  %120 = bitcast i8* %malloccall16 to i8****
  store i8**** %120, i8***** %9, align 8
  %121 = load i8****, i8***** %118, align 8
  store i8**** %121, i8***** %9, align 8
  %122 = load i8****, i8***** %9, align 8
  %123 = getelementptr i8***, i8**** %122, i64 -1
  store i8** %114, i8**** %123, align 8
  %124 = alloca i8**, align 8
  %malloccall17 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %125 = bitcast i8* %malloccall17 to i8**
  store i8** %125, i8*** %124, align 8
  %126 = load i8**, i8*** %124, align 8
  %127 = getelementptr i8*, i8** %126, i64 0
  %128 = load i8**, i8*** %124, align 8
  %129 = getelementptr i8*, i8** %128, i64 0
  %130 = load i8*, i8** %129, align 8
  %131 = alloca i8**, align 8
  %132 = load i8**, i8*** %124, align 8
  store i8** %132, i8*** %131, align 8
  %malloccall18 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %133 = bitcast i8* %malloccall18 to i8**
  store i8** %133, i8*** %124, align 8
  %134 = load i8**, i8*** %131, align 8
  store i8** %134, i8*** %124, align 8
  %135 = load i8**, i8*** %124, align 8
  %136 = getelementptr i8*, i8** %135, i64 0
  store i8* getelementptr inbounds ([5 x i8], [5 x i8]* @7, i32 0, i32 0), i8** %136, align 8
  %137 = load i8**, i8*** %124, align 8
  %138 = load i8****, i8***** %9, align 8
  %139 = getelementptr i8***, i8**** %138, i64 0
  %140 = load i8***, i8**** %139, align 8
  %141 = alloca i8****, align 8
  %142 = load i8****, i8***** %9, align 8
  store i8**** %142, i8***** %141, align 8
  %malloccall19 = tail call i8* @malloc(i64 ptrtoint (i1** getelementptr (i1*, i1** null, i32 1) to i64))
  %143 = bitcast i8* %malloccall19 to i8****
  store i8**** %143, i8***** %9, align 8
  %144 = load i8****, i8***** %141, align 8
  store i8**** %144, i8***** %9, align 8
  %145 = load i8****, i8***** %9, align 8
  %146 = getelementptr i8***, i8**** %145, i64 0
  store i8** %137, i8**** %146, align 8
  %147 = load i8****, i8***** %9, align 8
  %148 = getelementptr i8***, i8**** %147, i64 0
  %149 = load i8***, i8**** %148, align 8
  %150 = getelementptr i8**, i8*** %149, i64 0
  %151 = load i8**, i8*** %150, align 8
  %152 = getelementptr i8*, i8** %151, i64 2
  %153 = load i8*, i8** %152, align 8
  %calltmp = call double (i8*, ...) @printstr(i8* %153)
  ret void
}

declare noalias i8* @malloc(i64)

