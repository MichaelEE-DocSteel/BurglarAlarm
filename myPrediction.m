% function detected = myPrediction(testImage,sceneFeatures,nFaces)
% % Companion file for streamingFaceRecognition demo
% % Brett Shoelson, PhD
% % brett.shoelson@mathworks.com
% % 3/3/2015
% 
% % Copyright The MathWorks, Inc. 2015
% 
% fcnHandle = @(x) detectFASTFeatures(x,...
% 	'MinQuality',0.025,...
% 	'MinContrast',0.025); %#ok
% extractorMethod = 'SURF'; %#ok
% metric = 'SAD'; %#ok
% % adjustHistograms = false;
% % if adjustHistograms
% % 	testImage = histeq(testImage);
% % end
% %
% boxPoints = fcnHandle(testImage);
% [boxFeatures, boxPoints] = extractFeatures(testImage, boxPoints,...
% 	'Method',extractorMethod,...
% 	'BlockSize',3,...
% 	'SURFSize',64);
% matchMetric = zeros(size(boxFeatures,1),nFaces);
% for ii = 1:nFaces
% 	[~,matchMetric(:,ii)] = matchFeatures(boxFeatures,sceneFeatures{ii},...
% 		'MaxRatio',1,...
% 		'MatchThreshold',100,...
% 		'Metric',metric);
% end
% % if min(mean(matchMetric)) > 1.5
% % 	detected = 0;
% % else
% 	[~,detected] = min(mean(matchMetric));
% % end

function detected = myPrediction(testImage, sceneFeatures, nFaces)
% Function to detect faces from the dataset
% Inputs:
%   - testImage: Image to be tested for face detection
%   - sceneFeatures: Cell array containing features for faces in the dataset
%   - nFaces: Number of faces in the dataset
%
% Outputs:
%   - detected: The index of the detected face in the dataset, or 0 if no match is found

fcnHandle = @(x) detectFASTFeatures(x, ...
    'MinQuality', 0.025, ...
    'MinContrast', 0.025); %#ok
extractorMethod = 'SURF'; %#ok
metric = 'SAD'; %#ok

% Detect key points from the test image using FAST feature detector
boxPoints = fcnHandle(testImage);
[boxFeatures, boxPoints] = extractFeatures(testImage, boxPoints, ...
    'Method', extractorMethod, ...
    'BlockSize', 3, ...
    'SURFSize', 64);

% Initialize a matrix to store match metrics for each face in the dataset
matchMetric = zeros(size(boxFeatures,1), nFaces);

% Compare the features of the test image to each face in the dataset
for ii = 1:nFaces
    % Match features of the test image to the i-th face's features in the dataset
    [~, matchMetric(:, ii)] = matchFeatures(boxFeatures, sceneFeatures{ii}, ...
        'MaxRatio', 1, ...
        'MatchThreshold', 100, ...
        'Metric', metric);
end

% Compute the average match metric for each dataset face
meanMatchMetric = mean(matchMetric, 1);

% Find the index of the face with the minimum average match metric (i.e., best match)
[~, detectedIndex] = min(meanMatchMetric);

% Apply a threshold to reject poor matches (optional)
% Set a threshold to determine whether the match is strong enough
matchThreshold = 1.5;  % You can adjust this threshold based on your dataset

if meanMatchMetric(detectedIndex) > matchThreshold
    % No valid face detected from the dataset if the match is too weak
    detected = 0;
else
    % Return the index of the detected face in the dataset
    detected = detectedIndex;
end
