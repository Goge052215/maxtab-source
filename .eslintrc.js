module.exports = {
  root: true,
  env: {
    browser: true,
    es2021: true,
    node: true,
  },
  extends: [
    'eslint:recommended',
  ],
  parserOptions: {
    sourceType: 'module',
  },
  overrides: [
    {
      files: ['*.js'],
      parserOptions: {
        sourceType: 'module',
      },
    },
  ],
  rules: {
    'no-unused-vars': 'warn',
  },
  ignorePatterns: [
    'node_modules/**/*',
    'dist/**/*',
    'build/**/*',
    '*.json'
  ],
};