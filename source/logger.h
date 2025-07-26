#pragma once

#include "gravitybody.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using logCondition = std::function<bool(size_t)>;

class Logger {
private:
	std::ofstream outFile;
	size_t body;
	uint16_t captureData;
	std::vector<logCondition> conditions;
	bool enabled, isCSV;

	void writeLog(glm::float64 timeStamp);
public:
	Logger(const std::filesystem::path& filePath, size_t body, uint16_t data = DATA_NONE);
	~Logger();

	void addCondition(logCondition condition);
	void logIfNeeded(glm::float64 timeStamp);
	void enable();
	void disable();
};