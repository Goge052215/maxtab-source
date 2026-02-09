import configuration from '@system.configuration'
import en from '../i18n/en.json'
import zhCN from '../i18n/zh-CN.json'

const locales = {
  'zh-CN': zhCN,
  en
}

const getCurrentLang = () => {
  const locale = configuration.getLocale()
  if (locale && locale.language && locale.language.indexOf('zh') > -1) {
    return 'zh-CN'
  }
  return 'en'
}

const getLocaleStrings = (lang) => {
  const targetLang = lang || getCurrentLang()
  return locales[targetLang] || en
}

const getValue = (obj, path) => {
  if (!obj || !path) return undefined
  return path.split('.').reduce((acc, key) => (acc ? acc[key] : undefined), obj)
}

const t = (path, lang) => {
  const strings = getLocaleStrings(lang)
  const value = getValue(strings, path)
  if (value !== undefined && value !== null) return value
  const fallback = getValue(en, path)
  return fallback !== undefined && fallback !== null ? fallback : path
}

export default {
  getCurrentLang,
  getLocaleStrings,
  t
}
