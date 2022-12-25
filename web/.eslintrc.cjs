/* eslint-env node */

module.exports = {
  root: true,
  parser: "@typescript-eslint/parser",
  extends: ["preact", "plugin:@typescript-eslint/recommended"],
  ignorePatterns: ["dist/"],
  rules: {
    "@typescript-eslint/ban-ts-comment": [
      "error",
      {
        "ts-expect-error": "allow-with-description",
        "ts-ignore": "allow-with-description",
        "ts-nocheck": "allow-with-description",
        "ts-check": "allow-with-description",
        minimumDescriptionLength: 5,
      },
    ],
    "jest/no-deprecated-functions": 0,
  },
};
