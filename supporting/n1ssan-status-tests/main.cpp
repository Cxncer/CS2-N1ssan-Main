// n1ssan status protocol harness
//
// Validates the status records the DLL actually writes. Protocol 1 records carry
// seven fields; protocol 2 appends the shared failure classification code:
//
//   N1SSAN_STATUS|2|2026-11-13T09:00:00Z|4242|build-3|READY|reason=runtime_worker_heartbeat signatures=11/11|code=E_NONE
//
// The Launcher (DllStatusMonitor), the Bridge (bridge/diagnostics.ts), and this
// harness must accept exactly the same records.

#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct StatusRecord
{
    int protocol = 0;
    std::string timestamp;
    std::string processId;
    std::string buildId;
    std::string state;
    std::string reason;
    std::string code;
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
    return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
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

    const int year = NumberAt(s, 0, 4);
    const int month = NumberAt(s, 5, 2);
    const int day = NumberAt(s, 8, 2);
    const int hour = NumberAt(s, 11, 2);
    const int minute = NumberAt(s, 14, 2);
    const int second = NumberAt(s, 17, 2);

    if (year < 1 || month < 1 || month > 12)
        return false;

    if (hour > 23 || minute > 59 || second > 59)
        return false;

    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int maxDay = days[month - 1];

    if (month == 2 && IsLeapYear(year))
        maxDay = 29;

    return day >= 1 && day <= maxDay;
}

// Codes are the shared failure classification: E_* is a fault, W_* is a
// degraded-but-usable condition.
static bool ValidateCode(const std::string &code)
{
    if (code.size() < 3)
        return false;

    if (code[0] != 'E' && code[0] != 'W')
        return false;

    if (code[1] != '_')
        return false;

    for (size_t i = 2; i < code.size(); ++i)
    {
        const unsigned char c = static_cast<unsigned char>(code[i]);
        if (!std::isupper(c) && !std::isdigit(c) && c != '_')
            return false;
    }

    return true;
}

static bool ParseStatus(const std::string &line, StatusRecord &result)
{
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, '|'))
        fields.push_back(field);

    const bool protocolOne = fields.size() == 7;
    const bool protocolTwo = fields.size() == 8;

    if (!protocolOne && !protocolTwo)
        return false;

    if (fields[0] != "N1SSAN_STATUS")
        return false;

    if (fields[1] != "1" && fields[1] != "2")
        return false;

    // A protocol 2 record must carry its code; a protocol 1 record must not.
    if (protocolTwo != (fields[1] == "2"))
        return false;

    if (!ValidateTimestamp(fields[2]))
        return false;

    if (fields[3].empty() || !IsDigits(fields[3], 0, fields[3].size()))
        return false;

    if (fields[4].empty())
        return false;

    std::string reason = fields[6];

    if (reason.rfind("reason=", 0) != 0)
        return false;

    reason = reason.substr(7);

    if (reason.empty())
        return false;

    std::string code = "E_NONE";

    if (protocolTwo)
    {
        if (fields[7].rfind("code=", 0) != 0)
            return false;

        code = fields[7].substr(5);

        if (!ValidateCode(code))
            return false;
    }

    result.protocol = fields[1] == "2" ? 2 : 1;
    result.timestamp = fields[2];
    result.processId = fields[3];
    result.buildId = fields[4];
    result.state = fields[5];
    result.reason = reason;
    result.code = code;

    return true;
}

static bool ReasonStartsWith(const std::string &reason, const std::string &prefix)
{
    return reason.rfind(prefix, 0) == 0;
}

static bool ValidateStatusSemantics(const StatusRecord &record)
{
    if (record.state == "INITIALIZING")
        return record.reason == "worker_started";

    if (record.state == "CHECKS_PASSED")
        return record.reason == "required_initialization_checks_passed";

    if (record.state == "READY")
        return ReasonStartsWith(record.reason, "runtime_worker_heartbeat");

    if (record.state == "DEGRADED")
    {
        return ReasonStartsWith(record.reason,
                                "optional_glove_schema_unavailable") &&
               record.code[0] == 'W';
    }

    if (record.state == "FAILED")
    {
        static const std::vector<std::string> allowedReasons =
            {
                "minhook_load_failed",
                "client_module_unavailable",
                "bridge_directory_unavailable",
                "function_resolution_failed",
                "inventory_runtime_initialization_failed",
                "fsn_hook_installation_failed",
                "runtime_mode_worker_unavailable"};

        for (const auto &reason : allowedReasons)
            if (record.reason == reason)
                return record.code[0] == 'E';

        return false;
    }

    return false;
}

static bool ValidateLifecycle(const std::vector<std::string> &lines)
{
    // A complete lifecycle must contain exactly two records.
    if (lines.size() != 2)
        return false;

    StatusRecord first;
    StatusRecord second;

    if (!ParseStatus(lines[0], first))
        return false;

    if (!ParseStatus(lines[1], second))
        return false;

    if (!ValidateStatusSemantics(first) || !ValidateStatusSemantics(second))
        return false;

    if (first.state != "INITIALIZING")
        return false;

    if (second.state != "FAILED" && second.state != "CHECKS_PASSED")
        return false;

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
    const std::string v2Prefix = "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|";

    std::vector<TestCase> parseTests =
        {
            {"Protocol 1 INITIALIZING",
             "N1SSAN_STATUS|1|" + validTime + "|4242|build-3|INITIALIZING|reason=worker_started",
             true},
            {"Protocol 2 FAILED with code",
             v2Prefix + "FAILED|reason=minhook_load_failed|code=E_MINHOOK_LOAD",
             true},
            {"Protocol 2 READY with signature health",
             v2Prefix + "READY|reason=runtime_worker_heartbeat signatures=11/11|code=E_NONE",
             true},
            {"Wrong protocol prefix",
             "INVALID|1|" + validTime + "|4242|build-3|INITIALIZING|reason=worker_started",
             false},
            {"Unsupported protocol version",
             "N1SSAN_STATUS|3|" + validTime + "|4242|build-3|INITIALIZING|reason=worker_started",
             false},
            {"Protocol 2 without code",
             v2Prefix + "READY|reason=runtime_worker_heartbeat",
             false},
            {"Protocol 1 with code",
             "N1SSAN_STATUS|1|" + validTime + "|4242|build-3|READY|reason=runtime_worker_heartbeat|code=E_NONE",
             false},
            {"Malformed code",
             v2Prefix + "FAILED|reason=minhook_load_failed|code=minhook",
             false},
            {"Missing process id",
             "N1SSAN_STATUS|2|" + validTime + "||build-3|READY|reason=runtime_worker_heartbeat|code=E_NONE",
             false},
            {"Invalid timestamp format",
             "N1SSAN_STATUS|1|2026/11/02|4242|build-3|INITIALIZING|reason=worker_started",
             false},
            {"Invalid calendar date",
             "N1SSAN_STATUS|1|2026-02-30T08:00:00Z|4242|build-3|INITIALIZING|reason=worker_started",
             false},
            {"Invalid time",
             "N1SSAN_STATUS|1|2026-11-02T25:00:00Z|4242|build-3|INITIALIZING|reason=worker_started",
             false},
            {"Missing reason",
             "N1SSAN_STATUS|1|" + validTime + "|4242|build-3|FAILED|reason=",
             false},
            {"Extra field",
             "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=minhook_load_failed|code=E_MINHOOK_LOAD|extra",
             false},
            {"Valid leap day",
             "N1SSAN_STATUS|1|2024-02-29T08:00:00Z|4242|build-3|INITIALIZING|reason=worker_started",
             true}};

    struct LifecycleTest
    {
        std::string name;
        std::vector<std::string> records;
        bool expected;
    };

    const std::string init =
        "N1SSAN_STATUS|2|2026-11-02T08:00:00Z|4242|build-3|INITIALIZING|reason=worker_started|code=E_NONE";

    const std::string checksPassed =
        "N1SSAN_STATUS|2|2026-11-02T08:00:01Z|4242|build-3|CHECKS_PASSED|reason=required_initialization_checks_passed|code=E_NONE";

    const std::string failedRecord =
        "N1SSAN_STATUS|2|2026-11-02T08:00:01Z|4242|build-3|FAILED|reason=fsn_hook_installation_failed|code=E_FSN_HOOK";

    const std::string duplicateInit =
        "N1SSAN_STATUS|2|2026-11-02T08:00:01Z|4242|build-3|INITIALIZING|reason=duplicate|code=E_NONE";

    const std::string lateRecord =
        "N1SSAN_STATUS|2|2026-11-02T08:00:02Z|4242|build-3|FAILED|reason=late_failure|code=E_UNEXPECTED";

    const std::string earlierRecord =
        "N1SSAN_STATUS|2|2026-11-01T08:00:00Z|4242|build-3|CHECKS_PASSED|reason=required_initialization_checks_passed|code=E_NONE";

    std::vector<LifecycleTest> lifecycleTests =
        {
            {"Initialization to success", {init, checksPassed}, true},
            {"Initialization to failure", {init, failedRecord}, true},
            {"Duplicate initialization", {init, duplicateInit}, false},
            {"Terminal before initialization", {checksPassed, init}, false},
            {"Record after terminal", {init, checksPassed, lateRecord}, false},
            {"Missing terminal state", {init}, false},
            {"Backward timestamp", {init, earlierRecord}, false}};

    std::vector<std::string> validSemanticRecords =
        {
            init,
            checksPassed,
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|READY|reason=runtime_worker_heartbeat signatures=9/11|code=E_NONE",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|DEGRADED|reason=optional_glove_schema_unavailable signatures=11/11|code=W_OPTIONAL_FEATURE_UNAVAILABLE",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=minhook_load_failed|code=E_MINHOOK_LOAD",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=client_module_unavailable|code=E_CLIENT_MODULE",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=bridge_directory_unavailable|code=E_BRIDGE_DIRECTORY",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=function_resolution_failed|code=E_FUNCTION_RESOLUTION",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=inventory_runtime_initialization_failed|code=E_INVENTORY_RUNTIME",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=runtime_mode_worker_unavailable|code=E_MODE_WORKER"};

    std::vector<std::string> invalidSemanticRecords =
        {
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=unknown_failure|code=E_UNEXPECTED",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|INITIALIZING|reason=minhook_load_failed|code=E_NONE",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|CHECKS_PASSED|reason=worker_started|code=E_NONE",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|FAILED|reason=minhook_load_failed|code=W_OPTIONAL_FEATURE_UNAVAILABLE",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|READY|reason=ready|code=E_NONE",
            "N1SSAN_STATUS|2|" + validTime + "|4242|build-3|UNKNOWN|reason=runtime_worker_heartbeat|code=E_NONE"};

    int passedCount = 0;
    int failedCount = 0;

    auto report = [&](bool success, const std::string &name)
    {
        if (success)
        {
            ++passedCount;
            std::cout << "[PASS] ";
        }
        else
        {
            ++failedCount;
            std::cout << "[FAIL] ";
        }

        std::cout << name << "\n";
    };

    std::cout << "====================================\n";
    std::cout << "  N1SSAN STATUS PROTOCOL TEST v2\n";
    std::cout << "====================================\n\n";

    std::cout << "--- PARSE TESTS ---\n";
    for (const auto &test : parseTests)
    {
        StatusRecord record;
        report(ParseStatus(test.input, record) == test.expected, test.name);
    }

    std::cout << "\n--- LIFECYCLE TESTS ---\n";
    for (const auto &test : lifecycleTests)
        report(ValidateLifecycle(test.records) == test.expected, test.name);

    std::cout << "\n--- SEMANTIC TESTS ---\n";
    for (size_t i = 0; i < validSemanticRecords.size(); ++i)
    {
        StatusRecord record;
        const bool actual =
            ParseStatus(validSemanticRecords[i], record) &&
            ValidateStatusSemantics(record);
        report(actual, "Valid production record " + std::to_string(i + 1));
    }

    for (size_t i = 0; i < invalidSemanticRecords.size(); ++i)
    {
        StatusRecord record;
        const bool actual =
            ParseStatus(invalidSemanticRecords[i], record) &&
            ValidateStatusSemantics(record);
        report(!actual, "Rejected inconsistent record " + std::to_string(i + 1));
    }

    const size_t totalTests =
        parseTests.size() +
        lifecycleTests.size() +
        validSemanticRecords.size() +
        invalidSemanticRecords.size();

    std::cout << "\n====================================\n";
    std::cout << "TOTAL: " << passedCount << "/" << totalTests << "\n";
    std::cout << "FAILED: " << failedCount << "\n";
    std::cout << "====================================\n";

    return failedCount == 0 ? 0 : 1;
}
