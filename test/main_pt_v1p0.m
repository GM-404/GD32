% 软件名称：1TxR手势识别雷达点云工具
% 软件版本：V1.0.0
% 软件功能：提取原始特征
% 作者信息：张威（西电广研院）
% 开发日期：2025年10月22日

close all;
clear all;
clc;
addpath(genpath(pwd));
% 调用函数解析JSON文件
datafile = 'frame_3_data.json';

matrix = jsonToMatrix(datafile);
%% 雷达参数设置
N_rx = 2;
N_chirp = 64;
M_sample = 128;
rRes = 0.1;
lamda = 0.0125;
T_frame = 0.05;
vRes = lamda/2/T_frame;
rbinCut = 20; %% max:M_sample/2
dataFrame = vec2arrFrms(matrix, N_rx, N_chirp, M_sample);%% 雷达数据转+换
frmNum = size(dataFrame,1)
%% 信号检测算法参数设置
crEn= 0; % 0=不去直流，1=去直流
theta_scan = -90:0.1:90;
% CFAR参数设置
% refCells = [6, 6];    % 速度维单侧6个，距离维单侧6个参考单元
% guardCells = [5, 3];   % 速度维单侧2个，距离维单侧3个保护单元
refCells = [6, 6];    % 速度维单侧6个，距离维单侧6个参考单元
guardCells = [5, 3];   % 速度维单侧5个，距离维单侧3个保护单元
thresholdFactor = 3;   % 检测门限倍数
%% 路径设置
fileName = datafile;% 输入Data文件名
[path, name] = fileparts(fileName);
featherName = ['./feather_data/', name,'_CrEn',num2str(crEn)]; % 输出文件名
%% 雷达衍生参数设置
dt=(0:1:M_sample-1);
df = (0:1:M_sample/2-1);
dv=(-N_chirp/2:1:(N_chirp / 2 -1));% 快时间维（距离）频点分布
rWin = hamming(M_sample);%距离维窗序列
rWin = rWin./sum(rWin)*length(rWin);
%%rWin = ones(M_sample,1);%%% 矩形窗，等效未加窗！！！！！！！！！！！

vWin = hann(N_chirp);%速度维窗序列
vWin = vWin./sum(vWin)*length(vWin);
vWin = ones(N_chirp,1);%%% 矩形窗，等效未加窗！！！！！！！！！！！
%% 雷达变量声明
dataFft1d = zeros(N_chirp,M_sample/2,N_rx);
dataFft2d = zeros(N_chirp,rbinCut,N_rx);
tarRs = zeros(1,frmNum)-0.1;
tarVs = zeros(1,frmNum);
tarAs = zeros(1,frmNum);
tarPs = zeros(1,frmNum);
%% main
for ii=1
    atData = squeeze(dataFrame(ii,:, :, :));%2*64*128
    for lane=1:N_rx
        %% 1dfft
        % 计算1DFFT ,可选去直流
        for chirp = 1 : N_chirp
            chirpData = squeeze(atData(lane, chirp, :));%
            chirpData = chirpData .* rWin;
            rangFftOri = fft(chirpData, M_sample);% 快时间维（距离）FFT数据, FFT点数与chirp采样点数相同
            rangFft = rangFftOri(1:M_sample/2);
            dataFft1d(chirp,:,lane) = rangFft.';
        end
        if crEn ==1
            dataFft1d_mean = mean(dataFft1d);
            dataFft1d = dataFft1d - mean(dataFft1d);
        end
        %% 2dfft
        for mm = 1 : rbinCut
            rangFft = dataFft1d(:,mm,lane);
            rangFft = rangFft .* vWin;
            dopplerFftOri = fft(rangFft, N_chirp);%  FFT点数与chirp采样点数相同
            dopplerFft = fftshift(dopplerFftOri);
            dataFft2d(:,mm,lane) = dopplerFft.';
        end
    end
    %% CFAR
    absdata =abs(dataFft2d);
    ncidata = mean(absdata,3);
    dlmwrite('cfar_data_in.txt', ncidata);
    [detections ] = cfar2dWithInterpolation(ncidata, refCells, guardCells, thresholdFactor);
    % 打印CFAR检测结果
    if ~isempty(detections)
        disp(['------> Frame = ',num2str(ii),':'])
        disp(' Detected target information:');
        for i = 1:length(detections)
            fprintf('Target %d: Range=%d(%.2f), Velocity=%d(%.2f), Amplitude=%.2f, SNR=%.2fdB, NoiseEst=%.2f\n', ...
                i, ...
                detections(i).rangeIdx, detections(i).rangeFine, ...  % Range original index + interpolated index
                detections(i).velIdx, detections(i).velFine, ...      % Velocity original index + interpolated index
                detections(i).amplitude, ...                           % Amplitude
                detections(i).snr, ...                                 % Signal-to-noise ratio
                detections(i).noise);                                  % Noise estimate
        end
    end
    %% DBF
    if isempty(detections)
        disp(['Frame = ',num2str(ii)])
    end
    for i = 1:length(detections)
        velIdx = detections(i).velIdx;    % 速度索引（1基）
        rangeIdx = detections(i).rangeIdx;% 距离索引（1基）
        velFine = detections(i).velFine ;% 插值速度索引（1基）
        rangeFine= detections(i).rangeFine;% 插值距离索引（1基）
        power = db(detections(i).amplitude);
        snr = detections(i).snr;
        sig = squeeze(dataFft2d(velIdx, rangeIdx,:));
        [dbfPower, a] = DBF(sig, theta_scan);
        v = (velFine- N_chirp/2-1)*vRes;
        r = (rangeFine-1)*rRes;
        detections(i).r = r;
        detections(i).v = v;
        detections(i).a = a;
        detections(i).p = power;
        str = sprintf('Frame %d-->%d: r=%0.2f, v=%.2f, a=%.1f°, P=%.1fdB, snr=%.1fdB', ii,i,r ,v,a, power,snr);% 显示的标题
        disp(str)
    end

%     %% 目标筛选
%     rThTar = 0.5;
%     vThTar = 1.0;
%     maxAmpIdx = findMaxAmpInRange(detections, rThTar, vThTar);
%     if ~isempty(maxAmpIdx)
%         r = detections(maxAmpIdx).r;
%         v = detections(maxAmpIdx).v;
%         a = detections(maxAmpIdx).a;
%         p = detections(maxAmpIdx).p;
%         tarRs(ii) = r;
%         tarVs(ii) = v;
%         tarAs(ii) = a;
%         tarPs(ii) = p;
%     end
end


%% 工具函数
%% A为向量，长度为K(天线数)*N（chirp数）*M（采样点）
function AA = vec2arrFrms(aa, K, N, M)
fnum = size(aa,1);
AA = zeros(fnum,K,N,M);
for j=1:fnum
    a = aa(j,:);
    A = vec2arr(a, K, N, M);
    AA(j,:,:,:)=A;
end
end
%% A为向量，长度为K(天线数)*N（chirp数）*M（采样点）
function A = vec2arr(a, K, N, M)
% 第一步：完全复刻原函数的中间矩阵b的生成
b = reshape(a, [K*M, N]).';  % 得到N×(K*M)矩阵

% 第二步：将b按列拆分为K个N×M矩阵，并重组为K×N×M
% 先将b重塑为N×M×K，此时每个"页"对应原循环中的k
temp = reshape(b, [N, M, K]);

% 第三步：转置维度，得到与原函数循环赋值完全相同的结果
A = permute(temp, [3, 1, 2]);
end
%% 测角
function [P, theta_est] = DBF(X, theta_scan)
% 基于FFT的DBF测角
% 输入:
%   X - 阵列接收数据(N×M矩阵); %% 天线数为N
%   theta_scan - 扫描角度向量(度)
% 输出:
%   P - 每个角度的功率谱
%   theta_est - 估计的角度(度)
[N, ~] = size(X);
% 计算每个扫描角度的波束输出功率
P = zeros(size(theta_scan));
d_lambda = 0.5;%%- 阵元间距(波长)
k = 2 * pi;  % 波数
win = hamming(N);
win =win./sum(win)*length(win);
X(:,1) = X(:,1).*win;
for i = 1:length(theta_scan)
    theta_rad = theta_scan(i) * pi / 180;
    n = 0:N-1;
    a = exp(1i * k * d_lambda * n' * sin(theta_rad));  % 导向矢量
    P(i) = abs(a' * X(:,1));  % 波束输出功率
end
% 寻找峰值位置作为估计角度
[~, idx] = max(P);
theta_est = theta_scan(idx);
end


function [detectionInfo] = cfar2dWithInterpolation(data, refCells, guardCells, thresholdFactor)
    % 二维CFAR检测+二次插值（插值逻辑函数化，全程1基索引）
    % 输出参数包含：.rangeIdx, .velIdx, .amplitude, .snr,.noise, .rangeFine, .velFine（均为1基）
    
    % 第一步：执行CFAR检测（循环扩展法）
    detectionInfo = cfar2dCircularExtend(data, refCells, guardCells, thresholdFactor);
    if isempty(detectionInfo)
        return;findMaxAmpInRange
    end
    
    % 获取数据维度（1基）
    [velDim, rangeDim] = size(data);
    
    % 第二步：对每个目标调用插值函数进行亚单元估计
    for i = 1:length(detectionInfo)
        velIdx = detectionInfo(i).velIdx;    % 速度索引（1基）
        rangeIdx = detectionInfo(i).rangeIdx;% 距离索引（1基）
        
        % 速度维插值（调用独立函数）
        velFine = peakInterpolation(data, velIdx, rangeIdx, true, velDim);
        % 距离维插值（调用独立函数）
        rangeFine = peakInterpolation(data, rangeIdx, velIdx, false, rangeDim);
        
        % 存储插值结果
        detectionInfo(i).velFine = velFine;
        detectionInfo(i).rangeFine = rangeFine;
    end
end


% 独立的二次插值函数
function fineIdx = peakInterpolation(data, peakIdx, fixedIdx, isVelocityDim, dimSize)
    % 对峰值点进行二次插值，返回亚单元精度索引（1基）
    % 输入参数：
    %   data           - 原始数据矩阵（N×M，1基）
    %   peakIdx        - 峰值点在插值维度的索引（1基）
    %   fixedIdx       - 固定维度的索引（1基，如速度维插值时固定距离索引）
    %   isVelocityDim  - 逻辑值，true=速度维插值，false=距离维插值
    %   dimSize        - 插值维度的总长度（避免越界）
    % 输出参数：
    %   fineIdx        - 插值后的亚单元索引（1基）
    
    % 若维度长度不足3，直接返回原始索引
    if dimSize < 3
        fineIdx = peakIdx;
        return;
    end
    
    % 获取峰值点及左右相邻点的索引（1基，确保不越界）
    prevIdx = max(1, peakIdx - 1);  % 左邻点索引
    currIdx = peakIdx;              % 峰值点索引
    nextIdx = min(dimSize, peakIdx + 1);  % 右邻点索引
    
    % 提取三点的幅度值（根据插值维度选择索引方式）
    if isVelocityDim
        % 速度维插值：数据维度为（速度, 距离），固定距离索引
        A_prev = data(prevIdx, fixedIdx);  % 左邻点幅度
        A_curr = data(currIdx, fixedIdx);  % 峰值点幅度
        A_next = data(nextIdx, fixedIdx);  % 右邻点幅度
    else
        % 距离维插值：数据维度为（速度, 距离），固定速度索引
        A_prev = data(fixedIdx, prevIdx);  % 左邻点幅度
        A_curr = data(fixedIdx, currIdx);  % 峰值点幅度
        A_next = data(fixedIdx, nextIdx);  % 右邻点幅度
    end
    
    % 二次插值计算偏移量（公式推导见前文，确保分母非零）
    denominator = A_prev - 2*A_curr + A_next + eps;  % 加eps避免除零
    delta = 0.5 * (A_prev - A_next) / denominator;
    
    % 限制偏移量范围（[-0.5, 0.5]），避免异常值
    delta = max(-0.5, min(0.5, delta));
    
    % 计算亚单元精度索引（1基）
    fineIdx = peakIdx + delta;
end

function [detectionInfo] = cfar2dCircularExtend(data, refCells, guardCells, thresholdFactor)
    % 二维CFAR检测（循环移位扩展法处理边界，适用于速度-距离二维数据）
    % 输入参数：
    %   data           - N×M实数矩阵，N为速度维（已fftshift），M为距离维
    %   refCells       - 1×2向量，[速度维单侧参考单元数, 距离维单侧参考单元数]
    %   guardCells     - 1×2向量，[速度维单侧保护单元数, 距离维单侧保护单元数]
    %   thresholdFactor- 线性检测门限参数（倍数）
    % 输出参数：
    %   detectionInfo  - 结构体数组，包含：
    %                   .rangeIdx  - 距离索引（0基）
    %                   .velIdx    - 速度索引（0基）
    %                   .amplitude - 目标幅度
    %                   .snr       - 信噪比(dB)
    
    % 内部参数设置
    refCellSel = 'median';% 升排序后选择第3个参考单元
    peakGroupingEn = 1; 

    % 参数解析
    ref_vel = refCells(1);    % 速度维单侧参考单元数
    ref_range = refCells(2);  % 距离维单侧参考单元数
    guard_vel = guardCells(1);% 速度维单侧保护单元数
    guard_range = guardCells(2);% 距离维单侧保护单元数
    
    % 获取数据维度（速度维N，距离维M）
    [velDim, rangeDim] = size(data);
    detectionInfo = struct('rangeIdx', [], 'velIdx', [], 'amplitude', [], 'snr', [], 'noise', []);
    detCount = 0;
    noiseCalcCell = data(end-5:end,end-5:end);
    noiseEstm = mean(noiseCalcCell(:));
    % 第0个距离bin不检测（从距离索引1开始，对应MATLAB索引2）
    for rangeIdx = 2:rangeDim-1  % 距离维MATLAB索引（1基）
        for velIdx = 2:velDim-1  % 速度维MATLAB索引（1基）
            currentAmp = data(velIdx, rangeIdx);
            if currentAmp <= 2*noiseEstm  % 排除很小的值
                continue;
            end
            if peakGroupingEn == 1
                tmp = [data(velIdx-1, rangeIdx),data(velIdx+1, rangeIdx),data(velIdx, rangeIdx-1),data(velIdx, rangeIdx+1)];
                if currentAmp < max(tmp)
                    continue;
                end
            end
            % --------------------------
            % 1. 速度维（上下）参考单元范围
            % --------------------------
            upStart = velIdx - guard_vel - ref_vel;    % 上侧参考单元起始
            upEnd = velIdx - guard_vel - 1;            % 上侧参考单元结束
            downStart = velIdx + guard_vel + 1;        % 下侧参考单元起始
            downEnd = velIdx + guard_vel + ref_vel;    % 下侧参考单元结束
            
            % --------------------------
            % 2. 距离维（左右）参考单元范围
            % --------------------------
            leftStart = rangeIdx - guard_range - ref_range;  % 左侧参考单元起始
            leftEnd = rangeIdx - guard_range - 1;            % 左侧参考单元结束
            rightStart = rangeIdx + guard_range + 1;         % 右侧参考单元起始
            rightEnd = rangeIdx + guard_range + ref_range;   % 右侧参考单元结束
            
            % --------------------------
            % 3. 循环移位扩展处理边界（速度维）
            %    - 上边界不足：循环上移（使用下方数据填充）
            %    - 下边界不足：循环下移（使用上方数据填充）
            % --------------------------
            upRef = [];
            if upStart <= upEnd
                % 计算速度维有效索引（循环移位逻辑）
                upIndices = upStart:upEnd;
                % 超出上边界（<1）的索引转换为从底部循环获取（velDim + 超出值）
                upIndices(upIndices < 1) = velDim + upIndices(upIndices < 1);
                % 超出下边界（>velDim）的索引转换为从顶部循环获取（超出值 - velDim）
                upIndices(upIndices > velDim) = upIndices(upIndices > velDim) - velDim;
                upRef = data(upIndices, rangeIdx);  % 提取上侧参考单元
            end
            
            downRef = [];
            if downStart <= downEnd
                downIndices = downStart:downEnd;
                downIndices(downIndices < 1) = velDim + downIndices(downIndices < 1);
                downIndices(downIndices > velDim) = downIndices(downIndices > velDim) - velDim;
                downRef = data(downIndices, rangeIdx);  % 提取下侧参考单元
            end
            
            % --------------------------
            % 4. 循环移位扩展处理边界（距离维）
            %    - 左边界不足：循环左移（使用右侧数据填充）
            %    - 右边界不足：循环右移（使用左侧数据填充）
            % --------------------------
            leftRef = [];
            if leftStart <= leftEnd
                leftIndices = leftStart:leftEnd;
                % 超出左边界（<1）的索引转换为从右侧循环获取（rangeDim + 超出值）
                leftIndices(leftIndices < 1) = rangeDim + leftIndices(leftIndices < 1);
                % 超出右边界（>rangeDim）的索引转换为从左侧循环获取（超出值 - rangeDim）
                leftIndices(leftIndices > rangeDim) = leftIndices(leftIndices > rangeDim) - rangeDim;
                leftRef = data(velIdx, leftIndices);  % 提取左侧参考单元
            end
            
            rightRef = [];
            if rightStart <= rightEnd
                rightIndices = rightStart:rightEnd;
                rightIndices(rightIndices < 1) = rangeDim + rightIndices(rightIndices < 1);
                rightIndices(rightIndices > rangeDim) = rightIndices(rightIndices > rangeDim) - rangeDim;
                rightRef = data(velIdx, rightIndices);  % 提取右侧参考单元
            end
            
            % --------------------------
            % 5. 合并参考单元并估计噪声
            % --------------------------
            refCellsData = [mean(upRef), mean(downRef), mean(leftRef), mean(rightRef)];  % 合并上下左右参考单元的均值

            if isempty(refCellsData)
                continue;
            end

            if strcmp(refCellSel, 'median')
                noiseEst =median(refCellsData);
            else
                refCellsData = sort(refCellsData);
                noiseEst = refCellsData(refCellSel);  % 噪声功率估计
            end
            % --------------------------
            % 6. 检测判决与结果存储
            % --------------------------
            threshold = thresholdFactor * noiseEst;
            if currentAmp > threshold
                detCount = detCount + 1;
                % 计算信噪比（信号功率/噪声功率，dB形式）
                snr = 20 * log10( currentAmp / noiseEst);
                % 转换为0基索引输出
                detectionInfo(detCount).rangeIdx = rangeIdx ;  % 距离索引（1基）
                detectionInfo(detCount).velIdx = velIdx ;      % 速度索引（1基）
                detectionInfo(detCount).amplitude = currentAmp;
                detectionInfo(detCount).snr = snr;
                detectionInfo(detCount).noise = noiseEst;
            end
        end
    end
    
    % 无目标时返回空
    if detCount == 0
        detectionInfo = [];
    end
end

function dataMatrix = jsonToMatrix(filePath)
    % jsonToMatrix 读取JSON文件并转换为有符号8位整数矩阵
    %   输入: filePath - JSON数据文件的路径
    %   输出: dataMatrix - 解析后的int8类型二维矩阵
    
    % 读取JSON文件内容
    jsonData = fileread(filePath);
    
    % 解析JSON数据
    parsedData = jsondecode(jsonData);
    
    % 检查解析结果的类型
    if isstruct(parsedData)
        % 确定矩阵的行和列数
        numFrames = length(parsedData);
        % 使用点符号访问结构体数组元素
        numElements = length(parsedData(1).frame_data);
        
        % 初始化二维矩阵（使用int8类型存储有符号整数）
        dataMatrix = int8(zeros(numFrames, numElements));
        
        % 提取数据并转换为有符号8位整数
        for i = 1:numFrames
            % 获取当前帧的无符号数据
            uintData = parsedData(i).frame_data;
            
            % 转换为有符号8位整数：大于127的值减去256
            signedData = uintData;
            signedData(uintData > 127) = uintData(uintData > 127) - 256;
            
            % 存入矩阵（转换为int8类型）
            dataMatrix(i, :) = int8(signedData);
        end
        
        % 显示结果信息
        disp(['转换后的有符号8位整数矩阵size:', num2str(size(dataMatrix))]);
    else
        error('解析的JSON数据不是结构体数组');
    end
end
