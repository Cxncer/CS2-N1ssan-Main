
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cctype>

struct StatusRecord
{
    int version = 0;
    std::string timestamp;
    std::string state;
    std::string reason;
};

static bool IsDigits(const std::string &s, size_t start, size_t count)
{
    if (start + count > s.size())
        return false;

    for (size_t i = start; i < start + count; ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i])))
            return false;

    return true;
}

static int NumberAt(const std::string &s, size_t pos, size_t count)
{
    return std::stoi(s.substr(pos, count));
}

static bool IsLeapYear(int year)
{
    return (year % 400 == 0) ||
           (year % 4 == 0 && year % 100 != 0);
}

static bool ValidateTimestamp(const std::string &s)
{
    if (s.size() != 20)
        return false;

    if (s[4] != '-' || s[7] != '-' ||
        s[10] != 'T' || s[13] != ':' ||
        s[16] != ':' || s[19] != 'Z')
        return false;

    if (!IsDigits(s, 0, 4) ||
        !IsDigits(s, 5, 2) ||
        !IsDigits(s, 8, 2) ||
        !IsDigits(s, 11, 2) ||
        !IsDigits(s, 14, 2) ||
        !IsDigits(s, 17, 2))
        return false;

    int year = NumberAt(s, 0, 4);
    int month = NumberAt(s, 5, 2);
    int day = NumberAt(s, 8, 2);
    int hour = NumberAt(s, 11, 2);
    int minute = NumberAt(s, 14, 2);
    int second = NumberAt(s, 17, 2);

    if (year < 1 || month < 1 || month > 12)
        return false;

    if (hour > 23 || minute > 59 || second > 59)
        return false;

    static const int days[] =
        {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31};

    int maxDay = days[month - 1];

    if (month == 2 && IsLeapYear(year))
        maxDay = 29;

    return day >= 1 && day <= maxDay;
}

static bool ParseStatus(
    const std::string &line,
    StatusRecord &result)
{
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, '|'))
        fields.push_back(field);

    if (fields.size() != 5)
        return false;

    if (fields[0] != "N1SSAN_STATUS")
        return false;

    if (fields[1] != "1")
        return false;

    if (!ValidateTimestamp(fields[2]))
        return false;

    if (fields[3] != "INITIALIZING" &&
        fields[3] != "FAILED" &&
        fields[3] != "CHECKS_PASSED")
        return false;

    if (fields[4].rfind("reason=", 0) != 0)
        return false;

    std::string reason = fields[4].substr(7);

    if (reason.empty())
        return false;

    result.version = 1;
    result.timestamp = fields[2];
    result.state = fields[3];
    result.reason = reason;

    return true;
}

static bool ValidateStatusSemantics(const StatusRecord &record)
{
    if (record.state == "INITIALIZING")
        return record.reason == "worker_started";

    if (record.state == "CHECKS_PASSED")
        return record.reason ==
               "required_initialization_checks_passed";

    if (record.state == "FAILED")
    {
        static const std::vector<std::string> allowedReasons =
            {
                "minhook_load_failed",
                "client_module_unavailable",
                "glove_schema_resolution_failed",
                "function_resolution_failed",
                "loadout_empty",
                "fsn_hook_installation_failed"};

        for (const auto &reason : allowedReasons)
        {
            if (record.reason == reason)
                return true;
        }
    }

    return false;
}

static bool ValidateLifecycle(
    const std::vector<std::string> &lines)
{
    // A complete lifecycle must contain exactly two records.
    if (lines.size() != 2)
        return false;

    StatusRecord first;
    StatusRecord second;

    // Parse both records first.
    if (!ParseStatus(lines[0], first))
        return false;

    if (!ParseStatus(lines[1], second))
        return false;

    // Validate state/reason consistency.
    if (!ValidateStatusSemantics(first) ||
        !ValidateStatusSemantics(second))
    {
        return false;
    }

    // Initialization must always come first.
    if (first.state != "INITIALIZING")
        return false;

    // Exactly one terminal state is permitted.
    if (second.state != "FAILED" &&
        second.state != "CHECKS_PASSED")
        return false;

    // Timestamps must not move backwards.
    if (second.timestamp < first.timestamp)
        return false;

    return true;
}

struct TestCase
{
    std::string name;
    std::string input;
    bool expected;
};

int main()
{
    const std::string validTime = "2026-11-02T08:00:00Z";

    std::vector<TestCase> tests =
        {
            {"Valid INITIALIZING",
             "N1SSAN_STATUS|1|" + validTime +
                 "|INITIALIZING|reason=worker_started",
             true},
            {"Valid FAILED",
             "N1SSAN_STATUS|1|" + validTime +
                 "|FAILED|reason=minhook_load_failed",
             true},
            {"Valid CHECKS_PASSED",
             "N1SSAN_STATUS|1|" + validTime +
                 "|CHECKS_PASSED|reason=none",
             true},
            {"Wrong protocol prefix",
             "INVALID|1|" + validTime +
                 "|INITIALIZING|reason=test",
             false},
            {"Unsupported version",
             "N1SSAN_STATUS|2|" + validTime +
                 "|INITIALIZING|reason=test",
             false},
            {"Invalid timestamp format",
             "N1SSAN_STATUS|1|2026/11/02|" +
                 std::string("INITIALIZING|reason=test"),
             false},
            {"Invalid calendar date",
             "N1SSAN_STATUS|1|2026-02-30T08:00:00Z"
             "|INITIALIZING|reason=test",
             false},
            {"Unknown state",
             "N1SSAN_STATUS|1|" + validTime +
                 "|READY|reason=test",
             false},
            {"Missing reason",
             "N1SSAN_STATUS|1|" + validTime +
                 "|FAILED|reason=",
             false},
            {"Extra field",
             "N1SSAN_STATUS|1|" + validTime +
                 "|FAILED|reason=test|extra",
             false},
            {"Invalid time",
             "N1SSAN_STATUS|1|2026-11-02T25:00:00Z"
             "|INITIALIZING|reason=test",
             false},
            {"Valid leap day",
             "N1SSAN_STATUS|1|2024-02-29T08:00:00Z"
             "|INITIALIZING|reason=test",
             true}};

    int passed = 0;
    int failed = 0;

    const std::string init =
        "N1SSAN_STATUS|1|2026-11-02T08:00:00Z"
        "|INITIALIZING|reason=worker_started";

    const std::string successRecord =
        "N1SSAN_STATUS|1|2026-11-02T08:00:01Z"
        "|CHECKS_PASSED|reason=required_initialization_checks_passed";

    const std::string failedRecord =
        "N1SSAN_STATUS|1|2026-11-02T08:00:01Z"
        "|FAILED|reason=minhook_load_failed";

    const std::string duplicateInit =
        "N1SSAN_STATUS|1|2026-11-02T08:00:01Z"
        "|INITIALIZING|reason=duplicate";

    const std::string lateRecord =
        "N1SSAN_STATUS|1|2026-11-02T08:00:02Z"
        "|FAILED|reason=late_failure";

    const std::string earlierRecord =
        "N1SSAN_STATUS|1|2026-11-01T08:00:00Z"
        "|CHECKS_PASSED|reason=required_initialization_checks_passed";

    struct LifecycleTest
    {
        std::string name;
        std::vector<std::string> records;
        bool expected;
    };

    std::vector<LifecycleTest> lifecycleTests =
        {
            {"Initialization to success", {init, successRecord}, true},
            {"Initialization to failure", {init, failedRecord}, true},
            {"Duplicate initialization", {init, duplicateInit}, false},
            {"Terminal before initialization", {successRecord, init}, false},
            {"Record after terminal", {init, successRecord, lateRecord}, false},
            {"Missing terminal state", {init}, false},
            {"Backward timestamp", {init, earlierRecord}, false}};

    std::cout << "\n--- LIFECYCLE TESTS ---\n";
    for (const auto &test : lifecycleTests)
    {
        bool actual = ValidateLifecycle(test.records);
        bool success = actual == test.expected;

        if (success)
        {
            ++passed;
            std::cout << "[PASS] ";
        }
        else
        {
            ++failed;
            std::cout << "[FAIL] ";
        }

        std::cout << test.name << "\n";
    }

    std::cout << "\n--- SEMANTIC TESTS ---\n";

    std::vector<std::string> semanticRecords =
        {
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|INITIALIZING|reason=worker_started",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|CHECKS_PASSED|reason=required_initialization_checks_passed",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|FAILED|reason=minhook_load_failed",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|FAILED|reason=client_module_unavailable",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|FAILED|reason=glove_schema_resolution_failed",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|FAILED|reason=function_resolution_failed",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|FAILED|reason=loadout_empty",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|FAILED|reason=fsn_hook_installation_failed"};

    for (size_t i = 0; i < semanticRecords.size(); ++i)
    {
        StatusRecord record;
        bool actual =
            ParseStatus(semanticRecords[i], record) &&
            ValidateStatusSemantics(record);

        if (actual)
        {
            ++passed;
            std::cout << "[PASS] Valid production reason " << i + 1 << "\n";
        }
        else
        {
            ++failed;
            std::cout << "[FAIL] Valid production reason " << i + 1 << "\n";
        }
    }

    std::vector<std::string> invalidSemanticRecords =
        {
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|FAILED|reason=unknown_failure",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|INITIALIZING|reason=minhook_load_failed",
            "N1SSAN_STATUS|1|2026-11-02T08:00:00Z|CHECKS_PASSED|reason=worker_started"};

    for (size_t i = 0; i < invalidSemanticRecords.size(); ++i)
    {
        StatusRecord record;
        bool actual =
            ParseStatus(invalidSemanticRecords[i], record) &&
            ValidateStatusSemantics(record);

        if (!actual)
        {
            ++passed;
            std::cout << "[PASS] Rejected inconsistent reason " << i + 1 << "\n";
        }
        else
        {
            ++failed;
            std::cout << "[FAIL] Accepted inconsistent reason " << i + 1 << "\n";
        }
    }

    std::cout << "\n";
    std::cout << "====================================\n";
    std::cout << "  N1SSAN STATUS PROTOCOL TEST v1\n";
    std::cout << "====================================\n\n";

    for (const auto &test : tests)
    {
        StatusRecord record;

        bool actual = ParseStatus(test.input, record);
        bool success = actual == test.expected;

        if (success)
        {
            ++passed;
            std::cout << "[PASS] ";
        }
        else
        {
            ++failed;
            std::cout << "[FAIL] ";
        }

        std::cout << test.name << "\n";
    }

    const size_t totalTests =
        tests.size() +
        lifecycleTests.size() +
        semanticRecords.size() +
        invalidSemanticRecords.size();

    std::cout << "\n====================================\n";
    std::cout << "TOTAL: " << passed << "/" << totalTests << "\n";
    std::cout << "FAILED: " << failed << "\n";
    std::cout << "====================================\n";

    return failed == 0 ? 0 : 1;
}
